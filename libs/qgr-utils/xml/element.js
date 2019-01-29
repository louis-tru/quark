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
var node = require('./node');
var LiveNodeList = require('./node').LiveNodeList;
var parser = require('./parser');

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

var Element = util.class('Element', node.Node, {

	nodeType: node.ELEMENT_NODE,
	
	hasAttribute: function (name) {
		return this.getAttributeNode(name) != null;
	},
	
	getAttribute: function (name) {
		var attr = this.getAttributeNode(name);
		return attr && attr.value || '';
	},

	setAttribute: function (name, value) {
		var attr = this.ownerDocument.createAttribute(name);
		attr.value = attr.nodeValue = value + '';
		this.setAttributeNode(attr);
	},

	getAttributeNode: function (name) {
		return this.attributes.getNamedItem(name);
	},

	setAttributeNode: function (newAttr) {
		this.attributes.setNamedItem(newAttr);
	},

	removeAttributeNode: function (oldAttr) {
		this.attributes._removeItem(oldAttr);
	},

	removeAttribute: function (name) {
		var attr = this.getAttributeNode(name);
		attr && this.removeAttributeNode(attr);
	},

	hasAttributeNS: function (namespaceURI, localName) {
		return this.getAttributeNodeNS(namespaceURI, localName) != null;
	},

	getAttributeNS: function (namespaceURI, localName) {
		var attr = this.getAttributeNodeNS(namespaceURI, localName);
		return attr && attr.value || '';
	},

	setAttributeNS: function (namespaceURI, qualifiedName, value) {
		var attr = this.ownerDocument.createAttributeNS(namespaceURI, qualifiedName);
		attr.value = attr.nodeValue = value + '';
		this.setAttributeNode(attr);
	},

	getAttributeNodeNS: function (namespaceURI, localName) {
		return this.attributes.getNamedItemNS(namespaceURI, localName);
	},

	setAttributeNodeNS: function (newAttr) {
		this.attributes.setNamedItemNS(newAttr);
	},

	removeAttributeNS: function (namespaceURI, localName) {
		var attr = this.getAttributeNodeNS(namespaceURI, localName);
		attr && this.removeAttributeNode(attr);
	},

	getElementsByTagName: function (name) {
		return new LiveNodeList(this, function (node) {
			var ls = [];
			visitNode(node, function (node) {
				if (node.nodeType == node.ELEMENT_NODE && node.tagName == name)
					ls.push(node);
				return true;
			});
			return ls;
		});
	},

	getElementsByTagNameNS: function (namespaceURI, localName) {
		return new LiveNodeList(this, function (node) {
			var ls = [];
			visitNode(node, function (node) {
				if (node.nodeType == node.ELEMENT_NODE && 
				node.namespaceURI == namespaceURI && 
				node.localName == localName)
					ls.push(node);
				return true;
			});
			return ls;
		});
	},

	get innerXml () {
		return Array.toArray(this.childNodes).join('');
	},

	set innerXml (xml) {

		this.removeAllChild();
		if(xml){
			new parser.Parser().fragment(this.ownerDocument, this, xml);
		}
	}

});

exports.Element = Element;
exports.visitNode = visitNode;
