/* ***** BEGIN LICENSE BLOCK *****
 * Distributed under the BSD license:
 *
 * Copyright (c) 2015, xuewen.chu
 * All rights reserved.
 * 
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *     * Redistributions of source code must retain the above copyright
 *       notice, this list of conditions and the following disclaimer.
 *     * Redistributions in binary form must reproduce the above copyright
 *       notice, this list of conditions and the following disclaimer in the
 *       documentation and/or other materials provided with the distribution.
 *     * Neither the name of xuewen.chu nor the
 *       names of its contributors may be used to endorse or promote products
 *       derived from this software without specific prior written permission.
 * 
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL xuewen.chu BE LIABLE FOR ANY
 * DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 * ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 * 
 * ***** END LICENSE BLOCK ***** */

var util = require('../util');

var html_tag = /^(br|hr|input|frame|img|area|link|col|meta|area|base|basefont|param)$/i;
var html = /^html$/i;

function xmlEncoder(c) {
	return c == '<' && '&lt;' || c == '&' && '&amp;' || 
				 c == '"' && '&quot;' || '&#' + c.charCodeAt() + ';';
}

function findNSMap(self) {
	var el = self;

	while (el.nodeType !== exports.ELEMENT_NODE) {
		if (el.nodeType === exports.ATTRIBUTE_NODE) {
			el = el.ownerElement;
		} else {
			el = el.parentNode;
		}
	}
	return el._namespaceMap;
}

function serializeToString(node, buf) {
	switch (node.nodeType) {
		case exports.ELEMENT_NODE:
			var attrs = node.attributes;
			var len = attrs.length;
			var child = node.firstChild;
			var nodeName = node.tagName;
			buf.push('<', nodeName);
			for (var i = 0; i < len; i++) {
				serializeToString(attrs.item(i), buf);
			}
			if (child) {
				buf.push('>');
				while (child) {
					serializeToString(child, buf);
					child = child.nextSibling;
				}
				buf.push('</', nodeName, '>');
			} else {

				var doc = node.ownerDocument;
				var doctype = doc.doctype;
				if (doctype && html.test(doctype.name)) {

					if (html_tag.test(nodeName))
						buf.push(' />');
					else
						buf.push('></', nodeName, '>');
				}
				else
					buf.push(' />');
			}
			return;
		case exports.DOCUMENT_NODE:
		case exports.DOCUMENT_FRAGMENT_NODE:
			var child = node.firstChild;
			while (child) {
				serializeToString(child, buf);
				child = child.nextSibling;
			}
			return;
		case exports.ATTRIBUTE_NODE:
			return buf.push(' ', node.name, '="', node.value.replace(/[<&"]/g, xmlEncoder), '"');
		case exports.TEXT_NODE:
			return buf.push(node.data.replace(/[<&]/g, xmlEncoder)); //(?!#?[\w\d]+;)
		case exports.CDATA_SECTION_NODE:
			return buf.push('<![CDATA[', node.data, ']]>');
		case exports.COMMENT_NODE:
			return buf.push("<!--", node.data, "-->");
		case exports.DOCUMENT_TYPE_NODE:

			var pubid = node.publicId;
			var sysid = node.systemId;

			buf.push('<!DOCTYPE ', node.name);
			if (pubid) {
				buf.push(' PUBLIC "', pubid);
				if (sysid && sysid != '.') {
					buf.push('" "', sysid);
				}
				buf.push('">');
			} else if (sysid && sysid != '.') {
				buf.push(' SYSTEM "', sysid, '">');
			} else {
				var sub = node.internalSubset;
				if (sub) {
					buf.push(" [", sub, "]");
				}
				buf.push(">");
			}
			return;
		case exports.PROCESSING_INSTRUCTION_NODE:
			return buf.push("<?", node.nodeName, " ", node.data, "?>");
		case exports.ENTITY_REFERENCE_NODE:
			return buf.push('&', node.nodeName, ';');
			//case ENTITY_NODE:
			//case NOTATION_NODE:
		default:
			buf.push('??', node.nodeName);
	}
}

/*
* attributes;
* children;
*
* writeable properties:
* nodeValue,Attr:value,CharacterData:data
* prefix
*/
function update(self, el, attr) {

	var doc = self.ownerDocument || self;
	doc._inc++;
	if (attr) {
		if (attr.namespaceURI == 'http://www.w3.org/2000/xmlns/') {
			//update namespace
		}
	} else {//node
		//update childNodes
		var cs = el.childNodes;
		var child = el.firstChild;
		var i = 0;

		while (child) {
			cs[i++] = child;
			child = child.nextSibling;
		}
		cs.length = i;
	}
}

function cloneNode(doc, node, deep) {
	var node2 = new node.constructor();
	for (var n in node) {
		var v = node[n];
		if (typeof v != 'object') {
			if (v != node2[n]) {
				node2[n] = v;
			}
		}
	}
	if (node.childNodes) {
		node2.childNodes = new NodeList();
	}
	node2.ownerDocument = doc;
	switch (node2.nodeType) {
		case exports.ELEMENT_NODE:
			var attrs = node.attributes;
			var attrs2 = node2.attributes = new NamedNodeMap();
			var len = attrs.length
			attrs2._ownerElement = node2;
			for (var i = 0; i < len; i++) {
				attrs2.setNamedItem(cloneNode(doc, attrs.item(i), true));
			}
			break; ;
		case exports.ATTRIBUTE_NODE:
			deep = true;
	}
	if (deep) {
		var child = node.firstChild;
		while (child) {
			node2.appendChild(cloneNode(doc, child, deep));
			child = child.nextSibling;
		}
	}
	return node2;
}

var Node = exports.Node = util.class('Node', {
  
	firstChild: null,
	lastChild: null,
	previousSibling: null,
	nextSibling: null,
	attributes: null,
	parentNode: null,
	childNodes: null,
	ownerDocument: null,
	nodeValue: null,
	namespaceURI: null,
	prefix: null,
	localName: null,

	// Modified in DOM Level 2:
	insertBefore: function (newChild, refChild) {//raises
		var parentNode = this;

		var cp = newChild.parentNode;
		if (cp) {
			cp.removeChild(newChild); //remove and update
		}
		if (newChild.nodeType === exports.DOCUMENT_FRAGMENT_NODE) {
			var newFirst = newChild.firstChild;
			var newLast = newChild.lastChild;
		}
		else
			newFirst = newLast = newChild;

		if (refChild == null) {
			var pre = parentNode.lastChild;
			parentNode.lastChild = newLast;
		} else {
			var pre = refChild.previousSibling;
			newLast.nextSibling = refChild;;
			refChild.previousSibling = newLast;
		}
		if (pre)
			pre.nextSibling = newFirst;
		else
			parentNode.firstChild = newFirst;

		newFirst.previousSibling = pre;
		do
			newFirst.parentNode = parentNode;
		while (newFirst !== newLast && (newFirst = newFirst.nextSibling))

		update(this, parentNode);
	},

	replaceChild: function (newChild, oldChild) {//raises
		this.insertBefore(newChild, oldChild);
		if (oldChild) {
			this.removeChild(oldChild);
		}
	},

	removeAllChild: function () {
		var ns = this.childNodes;
		for (var i = 0, l = ns.length; i < l; i++) {
			ns[i].parentNode = null;
			delete ns[i];
		}
		this.firstChild = null;
		this.lastChild = null;

		update(this, this);
	},

	removeChild: function (oldChild) {
		var parentNode = this;
		var previous = null;
		var child = this.firstChild;

		while (child) {
			var next = child.nextSibling;
			if (child === oldChild) {
				oldChild.parentNode = null; //remove it as a flag of not in document
				if (previous)
					previous.nextSibling = next;
				else
					parentNode.firstChild = next;

				if (next)
					next.previousSibling = previous;
				else
					parentNode.lastChild = previous;
				update(this, parentNode);
				return child;
			}
			previous = child;
			child = next;
		}
	},

	appendChild: function (newChild) {
		return this.insertBefore(newChild, null);
	},
	hasChildNodes: function () {
		return this.firstChild != null;
	},
	cloneNode: function (deep) {
		return cloneNode(this.ownerDocument || this, this, deep);
	},
	// Modified in DOM Level 2:
	normalize: function () {
		var child = this.firstChild;
		while (child) {
			var next = child.nextSibling;
			if (next && next.nodeType == exports.TEXT_NODE && child.nodeType == exports.TEXT_NODE) {
				this.removeChild(next);
				child.appendData(next.data);
			} else {
				child.normalize();
				child = next;
			}
		}
	},
	// Introduced in DOM Level 2:
	isSupported: function (feature, version) {
		return this.ownerDocument.implementation.hasFeature(feature, version);
	},
	// Introduced in DOM Level 2:
	hasAttributes: function () {
		return this.attributes.length > 0;
	},
	lookupPrefix: function (namespaceURI) {
		var map = findNSMap(this)
		if (namespaceURI in map) {
			return map[namespaceURI]
		}
		return null;
	},
	// Introduced in DOM Level 3:
	isDefaultNamespace: function (namespaceURI) {
		var prefix = this.lookupPrefix(namespaceURI);
		return prefix == null;
	},
	// Introduced in DOM Level 3:
	lookupNamespaceURI: function (prefix) {
		var map = findNSMap(this)
		for (var n in map) {
			if (map[n] == prefix) {
				return n;
			}
		}
		return null;
	},

	toString: function () {
		var buf = [];
		serializeToString(this, buf);
		return buf.join('');
	}

});

var NODE_TYPE = {
	ELEMENT_NODE: 1,
	ATTRIBUTE_NODE: 2,
	TEXT_NODE: 3,
	CDATA_SECTION_NODE: 4,
	ENTITY_REFERENCE_NODE: 5,
	ENTITY_NODE: 6,
	PROCESSING_INSTRUCTION_NODE: 7,
	COMMENT_NODE: 8,
	DOCUMENT_NODE: 9,
	DOCUMENT_TYPE_NODE: 10,
	DOCUMENT_FRAGMENT_NODE: 11,
	NOTATION_NODE: 12
};

util.ext(Node.prototype, NODE_TYPE);
util.ext(exports, NODE_TYPE);

/**
	* @see http://www.w3.org/TR/2000/REC-DOM-Level-2-Core-20001113/core.html#ID-536297177
	* The NodeList interface provides the abstraction of an ordered collection of nodes, without defining or constraining how this collection is implemented. NodeList objects in the DOM are live.
	* The items in the NodeList are accessible via an integral index, starting from 0.
	*/

var NodeList = exports.NodeList = util.class('NodeList', {
	/**
		* The number of nodes in the list. The range of valid child node indices is 0 to length-1 inclusive.
		* @standard level1
		*/
	length: 0,
	/**
		* Returns the indexth item in the collection. If index is greater than or equal to the number of nodes in the list, this returns null.
		* @standard level1
		* @param index  unsigned long
		*   Index into the collection.
		* @return Node
		* 	The node at the indexth position in the NodeList, or null if that is not a valid index.
		*/
	item: function (index) {
		return this[index] || null;
	}
});

function update_live_node_list(self) {
	var inc = self._node.ownerDocument._inc;
	if (self._inc != inc) {
		var ls = self._refresh(self._node);
		var l = ls.length;

		self._length = l;
		for(var i = 0; i < l; i++)
			self[i] = ls[i];

		self._inc = inc;
	}
}

exports.LiveNodeList = util.class('LiveNodeList', NodeList, {

	_length: 0,

	get length() {
		update_live_node_list(this);
		return this._length;
	},
  
	constructor: function (node, refresh) {
		this._node = node;
		this._refresh = refresh;
	},

	item: function (index) {
		update_live_node_list(this);
		return this[index] || null;
	}
});

var CharacterData = exports.CharacterData = util.class('CharacterData', Node, {
  
	data: '',
  
	substringData: function (offset, count) {
		return this.data.substring(offset, offset + count);
	},
  
	appendData: function (text) {
		text = this.data + text;
		this.nodeValue = this.data = text;
		this.length = text.length;
	},
  
	insertData: function (offset, text) {
		this.replaceData(offset, 0, text);
	},
  
	deleteData: function (offset, count) {
		this.replaceData(offset, count, "");
	},
  
	replaceData: function (offset, count, text) {
		var start = this.data.substring(0, offset);
		var end = this.data.substring(offset + count);
		text = start + text + end;
		this.nodeValue = this.data = text;
		this.length = text.length;
	},
});

exports.Attr = util.class('Attr', CharacterData, {
	nodeType: node.ATTRIBUTE_NODE
});

exports.CDATASection = util.class('CDATASection', CharacterData, {
	nodeName: "#cdata-section",
	nodeType: Node.CDATA_SECTION_NODE
});

exports.Comment = util.class('Comment', CharacterData, {
	nodeName: "#comment",
	nodeType: Node.COMMENT_NODE
});

exports.DocumentFragment = util.class('DocumentFragment', Node, {
	nodeName: '#document-fragment'
});

exports.DocumentType = util.class('DocumentType', Node, {

	// Introduced in DOM Level 2:
	/**
		* constructor function
		* @constructor
		* @param {String}              qualifiedName
		* @param {String}              publicId
		* @param {String}              systemId
		*/
	constructor: function (qualifiedName, publicId, systemId) {// raises:INVALID_CHARACTER_ERR,NAMESPACE_ERR

		this.name = qualifiedName;
		this.nodeName = qualifiedName;
		this.publicId = publicId;
		this.systemId = systemId;
		// Introduced in DOM Level 2:
		//readonly attribute DOMString        internalSubset;

		//TODO:..
		//  readonly attribute NamedNodeMap     entities;
		//  readonly attribute NamedNodeMap     notations;
	},

	nodeType: Node.DOCUMENT_TYPE_NODE

});

exports.Entity = util.class('Entity', Node, {
	nodeType: Node.ENTITY_NODE
});

exports.EntityReference = util.class('EntityReference', Node, {
	nodeType: Node.ENTITY_REFERENCE_NODE
});

exports.Notation = util.class('Notation', Node, {
	nodeType: Node.NOTATION_NODE
});

exports.ProcessingInstruction = util.class('ProcessingInstruction', Node, {
	nodeType: Node.PROCESSING_INSTRUCTION_NODE
});

exports.Text = util.class('Text', CharacterData, {
	nodeName: "#text",
	nodeType: Node.TEXT_NODE,
	splitText: function (offset) {
		var text = this.data;
		var newText = text.substring(offset);
		text = text.substring(0, offset);
		this.data = this.nodeValue = text;
		this.length = text.length;
		var newNode = this.ownerDocument.createTextNode(newText);
		if (this.parentNode) {
			this.parentNode.insertBefore(newNode, this.nextSibling);
		}
		return newNode;
	}
});

