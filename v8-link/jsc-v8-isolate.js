
const ALL_PROPERTIES = 0;
const ONLY_WRITABLE = 1;
const ONLY_ENUMERABLE = 2;
const ONLY_CONFIGURABLE = 4;
const SKIP_STRINGS = 8;
const SKIP_SYMBOLS = 16;
function getPropertyNames(self, OwnOnly, property_filter, IncludeIndices) {
  var r = [];
  for (var i in self) {
    r.push(i);
  }
  return r;
}
function getOwnPropertyNames2(self, property_filter) {
  return Object.getOwnPropertyNames(self);
}
function isSymbol(val) {
  return typeof val == 'symbol';
}
function promiseState(promise) {
  return ({ pending: 0, rejected: 1, resolved: 2 })[promise.status] || 0;
}
function promiseCatch(promise, func) {
  return promise.catch(func);
}
function promiseThen(promise, func) {
  return promise.then(func);
}
function stringLength(str) {
  return str.length;
}
function typeOf(value) {
  return typeof value;
}
function valueOf(o) {
  return o.valueOf();
}

// reg exp flags
const kNone = 0,
kGlobal = 1 << 0,
kIgnoreCase = 1 << 1,
kMultiline = 1 << 2,
kSticky = 1 << 3,
kUnicode = 1 << 4,
kDotAll = 1 << 5;
function newRegExp(pattern, flags) {
  var flags_str = '';
  if (flags & kGlobal) flags_str += 'g';
  if (flags & kIgnoreCase) flags_str += 'i';
  if (flags & kMultiline) flags_str += 'm';
  if (flags & kSticky) flags_str += 'y';
  if (flags & kUnicode) flags_str += 'u';
  return new RegExp(pattern, flags_str);
}
function wrapFunctionScript(arguments, context_extensions_count, body) {
  var script;
  if (context_extensions_count) {
    var extensions = new Array(context_extensions_count);
    for (var i = 0; i < context_extensions_count; i++) {
      extensions[i] = '__a__' + i;
    }
    script =
    '(function __g('+ extensions.join(',') +'){with('+ extensions.join(',') + '){' +
      'return function __f('+arguments.join(',')+'){'+body+'}' +
    '}})';
  } else {
    script = '(function __f('+arguments.join(',')+'){'+body+'})';
  }
  return script;
}
function mapAsArray(map) {
  var r = [];
  for (var i of map)
    r.push(i[0], i[1]);
  return r;
}
function setAsArray(set) {
  var r = [];
  for (var i of map)
    r.push(i);
  return r;
}
function newPrivateValue(name) {
  return '__private_' + name;
}
function symbolName(symbol) {
  var mat = String(symbol).match(/\(([^\)]+)\)$/);
  if (mat) {
    return mat[1];
  }
}
function NativeToString() {
  var d = Object.getOwnPropertyDescriptor(this, 'toString');
  if (d && d.value === NativeToString) {
    return 'function() {\n  [native code]\n}';
  } else {
    return this.__proto__.toString.call(this);
  }
}
function stringConcat(left, right) {
  return left + right;
}

// exports
exports.getPropertyNames = getPropertyNames;
exports.getOwnPropertyNames = Object.getOwnPropertyNames;
exports.getOwnPropertyNames2 = getOwnPropertyNames2;
exports.getOwnPropertyDescriptor = Object.getOwnPropertyDescriptor;
exports.getOwnPropertyDescriptors = Object.getOwnPropertyDescriptors;
exports.defineProperty = Object.defineProperty;
exports.defineProperties = Object.defineProperties;
exports.hasOwnProperty = Object.hasOwnProperty;
exports.isSymbol = isSymbol;
exports.promiseState = promiseState;
exports.promiseCatch = promiseCatch;
exports.promiseThen = promiseThen;
exports.stringLength = stringLength;
exports.typeOf = typeOf;
exports.valueOf = valueOf;
exports.newRegExp = newRegExp;
exports.wrapFunctionScript = wrapFunctionScript;
exports.mapAsArray = mapAsArray;
exports.setAsArray = setAsArray;
exports.newPrivateValue = newPrivateValue;
exports.symbolName = symbolName;
exports.NativeToString = NativeToString;
exports.Function_prototype = Function.prototype;
exports.symbol_for_api = {};
exports.private_for_api = {};
exports.JSONStringify = JSON.stringify;
exports.JSONParse = JSON.parse;
exports.mapSet = Map.prototype.set;
exports.mapGet = Map.prototype.get;
exports.mapClear = Map.prototype.clear;
exports.mapHas = Map.prototype.has;
exports.mapDelete = Map.prototype.delete;
exports.setClear = Set.prototype.clear;
exports.setAdd = Set.prototype.add;
exports.setHas = Set.prototype.has;
exports.setDelete = Set.prototype.delete;
exports.symbolFor = Symbol.for;
exports.stringConcat = stringConcat;
