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

/**
  * Objects implementing the NamedNodeMap interface are used to 
  * represent collections of nodes that can be accessed by name. 
  * Note that NamedNodeMap does not inherit from NodeList; 
  * NamedNodeMaps are not maintained in any particular order. 
  * Objects contained in an object implementing NamedNodeMap 
  * may also be accessed by an ordinal index, but this is simply to 
  * allow convenient enumeration of the contents of a NamedNodeMap, 
  * and does not imply that the DOM specifies an order to these Nodes.
  * NamedNodeMap objects in the DOM are live.
  * used for attributes or DocumentType entities
  *
  */

var util = require('../util');
var exception = require('./exception');
var NodeList = require('./node').NodeList;

function findNodeIndex(self, node) {
	var i = self.length;
	while (i--) {
		if (self[i] == node) { return i }
	}
}

function add(self, node, old) {
	if (old) {
		self[findNodeIndex(self, old)] = node;
	} else {
		self[self.length++] = node;
	}
	var el = self._ownerElement;
	var doc = el && el.ownerDocument;
	if (doc)
		node.ownerElement = el;

	return old || null;
}


var NamedNodeMap = util.class('NamedNodeMap', NodeList, {

	getNamedItem: function (key) {

		var i = this.length;
		while (i--) {
			var node = this[i];
			if (node.nodeName == key)
				return node;
		}
	},
  
	setNamedItem: function (node) {
		var old = this.getNamedItem(node.nodeName);
		return add(this, node, old);
	},

	/* returns Node */
	setNamedItemNS: function (node) {
	  // raises: WRONG_DOCUMENT_ERR,NO_MODIFICATION_ALLOWED_ERR,INUSE_ATTRIBUTE_ERR
		var old = this.getNamedItemNS(node.namespaceURI, node.localName);
		return add(self, node, old);
	},

	_removeItem: function (node) {
		var i = this.length;
		var lastIndex = i - 1;
		while (i--) {
			var c = this[i];
			if (node === c) {
				var old = c;
				while (i < lastIndex) {
					this[i] = this[++i];
				}
				this.length = lastIndex;
				node.ownerElement = null;
				var el = this._ownerElement;
				var doc = el && el.ownerDocument;
				return old;
			}
		}
	},

	/* returns Node */
	removeNamedItem: function (key) {
		var node = this.getNamedItem(key);
		if (node) {
			this._removeItem(node);
		} else {
			throw exception.Exception(exception.NOT_FOUND_ERR, new Error());
		}
	}, // raises: NOT_FOUND_ERR,NO_MODIFICATION_ALLOWED_ERR

	//for level2
	getNamedItemNS: function (namespaceURI, localName) {
		var i = this.length;
		while (i--) {
			var node = this[i];
			if (node.localName == localName && node.namespaceURI == namespaceURI) {
				return node;
			}
		}
		return null;
	},

	removeNamedItemNS: function (namespaceURI, localName) {
		var node = this.getNamedItemNS(namespaceURI, localName);
		if (node) {
			this._removeItem(node);
		} else {
			throw exception.Exception(exception.NOT_FOUND_ERR, new Error());
		}
	},
});
