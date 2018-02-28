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

var node = require('./node');
var Element = require('./element').Element;
var NamedNodeMap = require('./named_node_map').NamedNodeMap;
var parser = require('./parser');

var Node_insertBefore = node.Node.members.insertBefore;
var Node_removeChild = node.Node.members.removeChild;

function visitNode(node, callback) {
	if (!callback(node))
		return false;
  
	var next = node.firstChild;
	if (next) {
		if (!visitNode(next, callback))
			return false;
	}
	if (next = node.nextSibling)
		return visitNode(next, callback);
	return true;
}

function importNode(doc, node, deep) {
	var node2;
	switch (node.nodeType) {
		case node.ELEMENT_NODE:
			node2 = node.cloneNode(false);
			node2.ownerDocument = doc;
			var attrs = node2.attributes;
			var len = attrs.length;
			for (var i = 0; i < len; i++) {
				node2.setAttributeNodeNS(importNode(doc, attrs.item(i), deep));
			}
		case node.DOCUMENT_FRAGMENT_NODE:
			break;
		case node.ATTRIBUTE_NODE:
			deep = true;
			break;
	}
	if (!node2) {
		node2 = node.cloneNode(false); //false
	}
	node2.ownerDocument = doc;
	node2.parentNode = null;
	if (deep) {
		var child = node.firstChild;
		while (child) {
			node2.appendChild(importNode(doc, child, deep));
			child = child.nextSibling;
		}
	}
	return node2;
}

var Document = util.class('Document', node.Node, {

	nodeName: '#document',
	nodeType: node.DOCUMENT_NODE,
	doctype: null,
	documentElement: null,
	_inc: 1,

	// Introduced in DOM Level 2:
	/**
		* constructor function
		* @constructor
		* @param {String}              namespaceURI
		* @param {String}              qualifiedName
		* @param {tesla.xml.DocumentType} doctype
		*/
	constructor: function (namespaceURI, qualifiedName, doctype) {
	  // raises:INVALID_CHARACTER_ERR,NAMESPACE_ERR,WRONG_DOCUMENT_ERR
		this.childNodes = new node.NodeList();
		this.doctype = doctype;
		if (doctype) {
			this.appendChild(doctype);
		}
		if (qualifiedName) {
			var root = this.createElementNS(namespaceURI, qualifiedName);
			this.appendChild(root);
		}
	},
  
	load: function (text) {
		new parser.Parser().fragment(this, null, text);
	},
  
	insertBefore: function (newChild, refChild) {//raises
		if (newChild.nodeType == node.DOCUMENT_FRAGMENT_NODE) {
			var child = newChild.firstChild;
			while (child) {
				this.insertBefore(newChild, refChild);
				child = child.nextSibling;
			}
			return newChild;
		}
		if (this.documentElement === null && newChild.nodeType == 1)
			this.documentElement = newChild;
    
		Node_insertBefore.call(this, newChild, refChild);
		newChild.ownerDocument = this;
		return newChild;
	},
  
	removeChild: function (oldChild) {
		if (this.documentElement == oldChild) {
			this.documentElement = null;
		}
		return Node_removeChild.call(this, newChild, refChild);
	},
  
	// Introduced in DOM Level 2:
	importNode: function (importedNode, deep) {
		return importNode(this, importedNode, deep);
	},
  
	// Introduced in DOM Level 2:
	getElementById: function (id) {
		var rtv = null;
		visitNode(this.documentElement, function (node) {
			if (node.nodeType == 1) {
				if (node.getAttribute('id') == id) {
					rtv = node;
					return false;
				}
				return true;
			}
		});
		return rtv;
	},
  
	getElementsByTagName: function (name) {
		var el = this.documentElement;
		return el ? el.getElementsByTagName(name) : [];
	},
  
	getElementsByTagNameNS: function (namespaceURI, localName) {
		var el = this.documentElement;
		return el ? el.getElementsByTagNameNS(namespaceURI, localName) : [];
	},
  
	//document factory method:
	createElement: function (tagName) {
		var node = new Element();
		node.ownerDocument = this;
		node.nodeName = tagName;
		node.tagName = tagName;
		node.childNodes = new node.NodeList();
		var attrs = node.attributes = new NamedNodeMap();
		attrs._ownerElement = node;
		return node;
	},
  
	createDocumentFragment: function () {
		var node = new node.DocumentFragment();
		node.ownerDocument = this;
		node.childNodes = new node.NodeList();
		return node;
	},
  
	createTextNode: function (data) {
		var node = new node.Text();
		node.ownerDocument = this;
		node.appendData(data);
		return node;
	},
  
	createComment: function (data) {
		var node = new node.Comment();
		node.ownerDocument = this;
		node.appendData(data);
		return node;
	},
  
	createCDATASection: function (data) {
		var node = new node.CDATASection();
		node.ownerDocument = this;
		node.appendData(data);
		return node;
	},
  
	createProcessingInstruction: function (target, data) {
		var node = new node.ProcessingInstruction();
		node.ownerDocument = this;
		node.target = target;
		node.data = data;
		return node;
	},
  
	createAttribute: function (name) {
		var node = new node.Attr();
		node.ownerDocument = this;
		node.name = name;
		node.nodeName = name;
		node.specified = true;
		return node;
	},

	createEntityReference: function (name) {
		var node = new node.EntityReference();
		node.ownerDocument = this;
		node.nodeName = name;
		return node;
	},

	// Introduced in DOM Level 2:
	createElementNS: function (namespaceURI, qualifiedName) {
		var node = new Element();
		var pl = qualifiedName.split(':');
		var attrs = node.attributes = new NamedNodeMap();
		node.childNodes = new node.NodeList();
		node.ownerDocument = this;
		node.nodeName = qualifiedName;
		node.tagName = qualifiedName;
		node.namespaceURI = namespaceURI;
		if (node.length == 2) {
			node.prefix = pl[0];
			node.localName = pl[1];
		} else {
			//el.prefix = null;
			node.localName = qualifiedName;
		}
		attrs._ownerElement = node;
		return node;
	},

	// Introduced in DOM Level 2:
	createAttributeNS: function (namespaceURI, qualifiedName) {
		var node = new node.Attr();
		var pl = qualifiedName.split(':');
		node.ownerDocument = this;
		node.nodeName = qualifiedName;
		node.name = qualifiedName;
		node.namespaceURI = namespaceURI;
		node.specified = true;
		if (pl.length == 2) {
			node.prefix = pl[0];
			node.localName = pl[1];
		} else {
			//el.prefix = null;
			node.localName = qualifiedName;
		}
		return node;
	},
});

module.exports = {

	Document: Document,

	/**
		* @func visitNode
		*/
	visitNode: visitNode
};
