import lldb


MAX_SYNTHETIC_CHILDREN = 256


def _raw(value):
    try:
        raw = value.GetNonSyntheticValue()
        return raw if raw.IsValid() else value
    except Exception:
        return value


def _child(value, *names):
    cur = _raw(value)
    for name in names:
        cur = cur.GetChildMemberWithName(name)
        if not cur.IsValid():
            return cur
    return cur


def _uint(value, default=0):
    try:
        return value.GetValueAsUnsigned(default)
    except Exception:
        return default


def _ptr(value):
    return _uint(value)


def _value_from_member(parent, name, member):
    if not member.IsValid():
        return None
    address = member.GetAddress().GetLoadAddress(parent.GetTarget())
    if address == lldb.LLDB_INVALID_ADDRESS:
        return member
    child = parent.CreateValueFromAddress(name, address, member.GetType())
    return child if child.IsValid() else member


def _canonical_type(value):
    value_type = _raw(value).GetType()
    canonical = value_type.GetCanonicalType()
    return canonical if canonical.IsValid() else value_type


def _template_arg(value, index):
    try:
        return _canonical_type(value).GetTemplateArgumentType(index)
    except Exception:
        return lldb.SBType()


def _matrix_summary(value, columns, rows):
    value = _raw(value)
    data = _child(value, "val")
    lines = []
    for row in range(rows):
        cells = []
        for column in range(columns):
            item = data.GetChildAtIndex(row * columns + column)
            cells.append(item.GetValue() if item.IsValid() and item.GetValue() else "?")
        lines.append("[%s]" % ", ".join(cells))
    # Xcode escapes newlines in summaries instead of rendering multiple lines.
    return "[%s]" % ", ".join(lines)


def mat_summary(value, _internal_dict):
    return _matrix_summary(value, 3, 2)


def mat4_summary(value, _internal_dict):
    return _matrix_summary(value, 4, 4)


def _array_ptr(value):
    return _child(value, "_ptr")


def _array_length(value):
    return _uint(_child(_array_ptr(value), "extra"))


def _array_capacity(value):
    return _uint(_child(_array_ptr(value), "capacity"))


def _array_data(value):
    return _child(_array_ptr(value), "val")


def array_summary(value, _internal_dict):
    value = _raw(value)
    length = _array_length(value)
    capacity = _array_capacity(value)
    data = _ptr(_array_data(value))
    shown = min(length, MAX_SYNTHETIC_CHILDREN)
    return "len=%u cap=%u shown=%u data=0x%x" % (length, capacity, shown, data)


def array_weak_summary(value, _internal_dict):
    return array_summary(value, _internal_dict)


class ArraySyntheticProvider:
    raw_children = ["_ptr"]

    def __init__(self, value, _internal_dict):
        self.value = value
        self.update()

    def update(self):
        self.value = _raw(self.value)
        self.ptr = _array_ptr(self.value)
        self.data = _array_data(self.value)
        self.length = _array_length(self.value)
        try:
            self.elem_type = self.data.GetType().GetPointeeType()
            self.elem_size = self.elem_type.GetByteSize()
        except Exception:
            self.elem_type = None
            self.elem_size = 0

    def num_children(self):
        return len(self.raw_children) + min(self.length, MAX_SYNTHETIC_CHILDREN)

    def has_children(self):
        return True

    def get_child_index(self, name):
        if name in self.raw_children:
            return self.raw_children.index(name)
        if name.startswith("[") and name.endswith("]"):
            try:
                item_index = int(name[1:-1])
                return len(self.raw_children) + item_index if item_index < MAX_SYNTHETIC_CHILDREN else -1
            except Exception:
                return -1
        return -1

    def get_child_at_index(self, index):
        if index < 0:
            return None
        if index < len(self.raw_children):
            return _child(self.value, self.raw_children[index])
        index -= len(self.raw_children)
        if index >= self.length or index >= MAX_SYNTHETIC_CHILDREN or not self.elem_type or self.elem_size == 0:
            return None
        address = _ptr(self.data) + index * self.elem_size
        return self.value.CreateValueFromAddress("[%u]" % index, address, self.elem_type)


def _list_length(value):
    return _uint(_child(value, "_length"))


def _list_first_node(value):
    return _child(value, "_end", "_next")


def list_summary(value, _internal_dict):
    value = _raw(value)
    length = _list_length(value)
    return "len=%u shown=%u" % (length, min(length, MAX_SYNTHETIC_CHILDREN))


class _LinkedNodeSyntheticProvider:
    next_field = "_next"
    data_after_field = "_next"
    raw_children = []

    def __init__(self, value, _internal_dict):
        self.value = value
        self.update()

    def update(self):
        self.value = _raw(self.value)
        self.length = self._length()
        self.node = self._first_node()
        self.node_type = self.node.GetType().GetPointeeType() if self.node.IsValid() else None
        self.data_type = self._data_type()

    def _length(self):
        return 0

    def _first_node(self):
        return lldb.SBValue()

    def _data_type(self):
        if not self.node_type:
            return None
        try:
            data_type = self.node_type.GetDirectNestedTypeWithName("Data")
            return data_type if data_type.IsValid() else None
        except Exception:
            return None

    def num_children(self):
        return len(self.raw_children) + min(self.length, MAX_SYNTHETIC_CHILDREN)

    def has_children(self):
        return True

    def get_child_index(self, name):
        if name in self.raw_children:
            return self.raw_children.index(name)
        if name.startswith("[") and name.endswith("]"):
            try:
                item_index = int(name[1:-1])
                return len(self.raw_children) + item_index if item_index < MAX_SYNTHETIC_CHILDREN else -1
            except Exception:
                return -1
        return -1

    def _node_at_index(self, index):
        node = self.node
        for _ in range(index):
            node = _child(node.Dereference(), self.next_field)
            if not node.IsValid() or _ptr(node) == 0:
                return lldb.SBValue()
        return node

    def get_child_at_index(self, index):
        if index < 0:
            return None
        if index < len(self.raw_children):
            return _child(self.value, self.raw_children[index])
        index -= len(self.raw_children)
        if index >= self.length or index >= MAX_SYNTHETIC_CHILDREN or not self.data_type:
            return None
        node = self._node_at_index(index)
        if not node.IsValid() or _ptr(node) == 0:
            return None
        node_value = node.Dereference()
        value = self._node_data_child(node_value, "[%u]" % index)
        if value is not None:
            return value
        after = _child(node_value, self.data_after_field)
        address = after.GetAddress().GetLoadAddress(self.value.GetTarget()) + after.GetType().GetByteSize()
        return self.value.CreateValueFromAddress("[%u]" % index, address, self.data_type)

    def _node_data_child(self, node_value, name):
        return None


class ListSyntheticProvider(_LinkedNodeSyntheticProvider):
    next_field = "_next"
    data_after_field = "_next"
    raw_children = ["_allocator", "_length", "_end"]

    def _length(self):
        return _list_length(self.value)

    def _first_node(self):
        return _list_first_node(self.value)

    def _data_type(self):
        return _template_arg(self.value, 0)

    def _node_data_child(self, node_value, name):
        value = _child(node_value, "value")
        return _value_from_member(self.value, name, value) if value.IsValid() else None


def _dict_length(value):
    return _uint(_child(value, "_length"))


def _dict_capacity(value):
    return _uint(_child(value, "_capacity"))


def _dict_first_node(value):
    return _child(value, "_end", "_next")


def dict_summary(value, _internal_dict):
    value = _raw(value)
    length = _dict_length(value)
    return "len=%u cap=%u shown=%u" % (length, _dict_capacity(value), min(length, MAX_SYNTHETIC_CHILDREN))


class DictSyntheticProvider(_LinkedNodeSyntheticProvider):
    next_field = "_next"
    data_after_field = "_conflict"
    raw_children = ["_allocator", "_indexed", "_length", "_capacity", "_end"]

    def _length(self):
        return _dict_length(self.value)

    def _first_node(self):
        return _dict_first_node(self.value)

    def _data_type(self):
        try:
            dict_type = _canonical_type(self.value)
            pair_type = dict_type.GetDirectNestedTypeWithName("Pair")
            if pair_type.IsValid():
                return pair_type
        except Exception:
            pass

        key_type = _template_arg(self.value, 0)
        value_type = _template_arg(self.value, 1)
        if not key_type.IsValid() or not value_type.IsValid():
            return None

        pair_name = "qk::Pair<%s, %s>" % (key_type.GetName(), value_type.GetName())
        pair_type = self.value.GetTarget().FindFirstType(pair_name)
        if pair_type.IsValid():
            return pair_type

        try:
            data_type = self.node_type.GetDirectNestedTypeWithName("Data")
            return data_type if data_type.IsValid() else None
        except Exception:
            return None

    def _node_data_child(self, node_value, name):
        pair = _child(node_value, "pair")
        return _value_from_member(self.value, name, pair) if pair.IsValid() else None

    def get_child_at_index(self, index):
        if index < 0:
            return None
        if index < len(self.raw_children):
            return _child(self.value, self.raw_children[index])
        index -= len(self.raw_children)
        if index >= self.length:
            return None

        node = self._node_at_index(index)
        if not node.IsValid() or _ptr(node) == 0:
            return None
        node_value = node.Dereference()
        pair = _child(node_value, "pair")
        if pair.IsValid():
            return _value_from_member(self.value, "[%u]" % index, pair)
        return node_value


def _iterator_ptr(value):
    return _child(value, "_ptr")


def iterator_summary(value, _internal_dict):
    value = _raw(value)
    return "ptr=0x%x" % _ptr(_iterator_ptr(value))


class SimpleIteratorSyntheticProvider:
    raw_children = ["_ptr"]

    def __init__(self, value, _internal_dict):
        self.value = value
        self.update()

    def update(self):
        self.value = _raw(self.value)
        self.ptr = _iterator_ptr(self.value)
        self.data_type = self.ptr.GetType().GetPointeeType() if self.ptr.IsValid() else None

    def num_children(self):
        return 2

    def has_children(self):
        return True

    def get_child_index(self, name):
        if name == "_ptr":
            return 0
        if name == "value":
            return 1
        return -1

    def get_child_at_index(self, index):
        if index == 0:
            return self.ptr
        if index == 1 and self.ptr.IsValid() and _ptr(self.ptr) != 0 and self.data_type:
            return self.value.CreateValueFromAddress("value", _ptr(self.ptr), self.data_type)
        return None


class ComplexIteratorSyntheticProvider(SimpleIteratorSyntheticProvider):
    def update(self):
        self.value = _raw(self.value)
        self.ptr = _iterator_ptr(self.value)

    def get_child_at_index(self, index):
        if index == 0:
            return self.ptr
        if index != 1 or not self.ptr.IsValid() or _ptr(self.ptr) == 0:
            return None

        node_value = self.ptr.Dereference()
        pair = _child(node_value, "pair")
        if pair.IsValid():
            return _value_from_member(self.value, "value", pair)
        value = _child(node_value, "value")
        if value.IsValid():
            return _value_from_member(self.value, "value", value)
        return node_value


def _string_length(value):
    value = _raw(value)
    expr = "((%s*)%s)->length()" % (value.GetTypeName(), value.GetLoadAddress())
    result = value.GetTarget().EvaluateExpression(expr)
    if result.IsValid() and result.GetError().Success():
        return _uint(result)
    short_len = _child(value, "_val", "s", "length")
    return _uint(short_len)


def _string_data_address(value):
    value = _raw(value)
    expr = "(const void*)((%s*)%s)->c_str()" % (value.GetTypeName(), value.GetLoadAddress())
    result = value.GetTarget().EvaluateExpression(expr)
    if result.IsValid() and result.GetError().Success():
        return _ptr(result)
    return 0


def _string_short_data(value, length):
    value = _raw(value)
    short_val = _child(value, "_val", "s", "val")
    if not short_val.IsValid():
        return None
    address = short_val.GetAddress().GetLoadAddress(value.GetTarget())
    if address == lldb.LLDB_INVALID_ADDRESS:
        return None
    process = value.GetProcess()
    error = lldb.SBError()
    data = process.ReadMemory(address, min(length, 32), error)
    return data if error.Success() else None


def _format_string_data(data, length):
    try:
        text = data.decode("utf-8", "replace")
    except Exception:
        text = repr(data)
    if length > len(data):
        text += "..."
    return text.replace("\n", "\\n").replace("\r", "\\r")


def string_summary(value, _internal_dict):
    value = _raw(value)
    length = _string_length(value)
    if length <= 32:
        data = _string_short_data(value, length)
        if data is not None:
            return 'len=%u "%s"' % (length, _format_string_data(data, length))

    address = _string_data_address(value)
    if address == 0:
        return "len=%u" % length

    process = value.GetProcess()
    error = lldb.SBError()
    size = min(length, 256)
    data = process.ReadMemory(address, size, error)
    if not error.Success():
        return "len=%u data=0x%x" % (length, address)

    return 'len=%u "%s"' % (length, _format_string_data(data, length))


def __lldb_init_module(debugger, _internal_dict):
    # Commands live in .lldbinit-Xcode. This hook exists so manual
    # `command script import .ide/qk_lldb.py` is also harmless.
    pass
