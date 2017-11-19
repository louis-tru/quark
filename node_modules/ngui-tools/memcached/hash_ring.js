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
var Bisection = require('bisection').Bisection;
var createHash = require('crypto').createHash;
var StringDecoder = require('string_decoder').StringDecoder;

/*
 * Generates the hash ring distribution based on the nodes and their weights
 *
 * @todo remove the filty var stack.
 */
function generateRing(self) {
  var totalweight = 0;
  var len;
  var i = len = self.nodes.length;
  var tmp;
  var node;
	var weight;
  var factor;
  var j;
  var k;
  var key;

  // Generate the total weight of all the nodes, each node weights 1 by default
  while (i--) {
    tmp = self.weights[self.nodes[i]];
    totalweight += (tmp || 1);
  }

  // Calculate our hash-ring
  for (i = 0; i < len; i++) {

    weight = 1;
    node = self.nodes[i];

    if (tmp = self.weights[node])
      weight = tmp;

    // The factor is based on the weight, the more weight the more space a item will get in our
    // hash ring
    factor = Math.floor((self.vnode_count * len * weight) / totalweight);

    for (j = 0; j < factor; j++) {

      tmp = self.hashKey(node + '-' + j);
      for (k = 0; k < 3; k++) {
        key = self.hashValue(tmp, function (x) { return x + k * 4 });
        self.ring[key] = node;
        self.sortedKeys.push(key);
      }
    }

  }

  // Sort the keys, nummeric !important. I forgot it at first and took me 2 hours to debug \o/
  self.sortedKeys.sort(function (a, b) { return a - b });
}


var HashRing = exports.HashRing = util.class('HashRing', {

  //public:
  vnode_count: 40,
  algorithm: 'crc32',

  /**
   * Creates a hashring for key => server lookups. It uses `crc32` as default algorithm
   * as it creates nice dense key / server distribution. A other algorithm that could be used
   * is `MD5` or `SHA1`. But take in account that the hashing can take allot time when choosing
   * the algorithm. The JavaScript based `crc32` algorithm beats `MD5` in performance.
   *
   * @param {String[]} nodes              arg The server / weights / servers for the hashring
   * @param {Object}   weights (Optional) The algorithm that should be used to hash the keys.
   * @constructor
   */
  constructor: function (nodes, weights) {

    this.ring = {};
    this.cache = {};
    this.sortedKeys = [];

    var w =
    this.weights = {};
    this.nodes = nodes;

    if (weights) {

      for (var i = 0, l = weights.length; i < l; i++)
        w[nodes[i]] = weights[i];
    }
    generateRing(this);
  },

  /**
   * Adds a server and regenerates the ring
   *
   * @todo create the same arguments interface as the constructor. 
   * @param {String} key    
   * @param {Number} weight (Optional)
   */
  add: function (key, weight) {
    if (this.nodes.indexOf(server) !== -1) return; // prevents duplicates

    // add weights 
    if (weights)
      this.weights[key] = weight;

    // pushes one, or more servers to
    this.nodes.push(server);
    this.sortedKeys.length = 0;

    // clear all old caches and regenerate
    this.ring = {};
    this.cache = {};
    generateRing(this);
  },

  /**
   * Removes a server from the ring and regenerates the ring.
   *
   * @param {String} key The IP of the server that needs to be removed
   */
  remove: function (key) {
    var index = this.nodes.indexOf(key);
    if (index == -1) return; // no need to re-generate the whole ring, no match

    this.nodes.splice(index, 1);
    if (this.weights[key])
      delete this.weights[key];

    // clear all old caches and regenerate
    this.ring = {};
    this.cache = {};
    generateRing(this);
  },

  /**
   * Replaces a assigned server of the ring with a new server
   * hot swapping servers
   *
   * @param {String} oldKey The server that needs to be replaced
   * @param {String} newKey The new server
   * @api public
   */
  replace: function (oldKey, newKey) {
    var self = this;

    // replace the servers
    util.keys(this.ring).forEach(function (key) {
      if (self.ring[key] == oldKey)
        self.ring[key] = newKey;
    });

    // update the cache, because we don't want to completly kill our cache
    // as other key -> server lookups might still be valid
    util.keys(this.cache).forEach(function (key) {
      if (self.cache[key] == oldKey)
        self.cache[key] = newKey;
    });

    // remove the server from this.nodes and replace it with new server as well
    this.nodes.splice(this.nodes.indexOf(oldKey), 1, newKey);
  },

  /**
   * returns the correct node for the key based on the hashing, or false if it fails to get
   * the node
   *
   * @param {String} key return
   * @returns {String|Boolean} False if there isn't a match, string of the server ip when we have a match
   * @api public
   */
  getNode: function (key) {
    // faster loookups
    if (this.cache[key]) return this.cache[key];

    var position = this.getNodePosition(key);

    // needs explicit checking for `false` as we can have position of 0
    var node = position === false ? false : this.ring[this.sortedKeys[position]];

    if (!node) return false;
    return this.cache[key] = node;
  },

  /**
   * Returns the position of the key inside the keyring
   *
   * @param {String} key The key that needs his it's position calculated.
   * @returns {Number} the index of sortedKeys
   * @api public
   */
  getNodePosition: function (key) {
    if (!this.sortedKeys.length) {
      return false;
    }

    var keys = this.generateKey(key);
    var nodes = this.sortedKeys;
    var position = Bisection.right(nodes, keys);

    return position === nodes.length ? 0 : position;
  },

  /**
   * Iterates over the nodes for a give key, can be used to create redundancy support.
   *
   * @param {String} key The key that should get a range
   * @param {Number} size The maxium amount of nodes to return
   * @param {Boolean} distinct Remove duplicate nodes
   * @returns {Array} The nodes
   * @api public
   */
  createRange: function (key, size, distinct) {
    if (!Object.keys(this.ring).length) return false;

    distinct = distinct === 'undefined' ? true : distinct;

    var returnvalues = {};
    var returnnodes = [];
    var position = this.getNodePosition(key);
    var slices = this.sortedKeys.slice(position);
    var value;
    var i = 0;
    var length = slices.length;

    // a small filter function that checks for duplicates
    function distinctFilter(value) {
      if (!returnvalues[value]) {
        returnvalues[value] = true;
        return value;
      }
    }

    for (; i < length; i++) {
      value = distinct ? distinctFilter(this.ring[slices[i]]) : this.ring[slices[i]];
      if (value) returnnodes.push(value);
      if (size && returnnodes.length >= size) break;
    };

    // as we might have reached the end of our sortedKeys array, and didn't fill our returnnodes completely:
    if (!size || returnnodes.length < size) {
      for (i = 0, length = this.sortedKeys.length; i < length; i++) {
        if (i < position) {
          value = distinct ? distinctFilter(this.ring[this.sortedKeys[i]]) : this.ring[this.sortedKeys[i]];
          if (value) returnnodes.push(value);
          if (size && returnnodes.length >= size) break;
        } else {
          break;
        }
      }
    }

    // now that we have collect all the nodes, we can return the range
    return returnnodes;

  },

  /**
   * Generates a long value of the key that represents a place on the hash ring.
   *
   * @param {String} hash The hash that needs to transformed to a long value
   * @returns {String} The hash value
   * @api public
   */
  generateKey: function (hash) {
    return this.hashValue(this.hashKey(hash), function (x) { return x });
  },

  /**
   * Changes the returned key to a `long value` by using the compare function
   *
   * @param {String} key The key that needs to be calculated
   * @param {Function} compare The calculation function
   * @returns {String} The hash value
   * @api public
   */
  hashValue: function (key, compare) {
    return (
      (key[compare(3)] << 24) |
      (key[compare(2)] << 16) |
      (key[compare(1)] << 8) |
      key[compare(0)]
     );
  },

  /**
   * Generates a hash from a key
   *
   * @param {String} The key that needs to be transformed in to a hash.
   * @returns {String} The hashed key
   * @api public
   */
  hashKey: function (key) {
    return this.algorithm == 'crc32' ?
      this.crc32HashKey(key) :
      createHash(this.algorithm)
        .update(key)
        .digest('hex')
        .split('')
        .map(function (v) { return v.charCodeAt(0) });
  },

  /**
   * Generates a crc32 value of a string.
   *
   * @param {String} str The string that needs to converted in to a hash.
   * @returns {String} The hash
   * @api public
   */
  crc32HashKey: function (str) {
    str = new StringDecoder('utf8').write(str);

    var crc = 0 ^ (-1);
    var i = 0;
    var length = str.length;
    var map = '00000000 77073096 EE0E612C 990951BA 076DC419 706AF48F \
E963A535 9E6495A3 0EDB8832 79DCB8A4 E0D5E91E 97D2D988 09B64C2B 7EB17CBD \
E7B82D07 90BF1D91 1DB71064 6AB020F2 F3B97148 84BE41DE 1ADAD47D 6DDDE4EB \
F4D4B551 83D385C7 136C9856 646BA8C0 FD62F97A 8A65C9EC 14015C4F 63066CD9 \
FA0F3D63 8D080DF5 3B6E20C8 4C69105E D56041E4 A2677172 3C03E4D1 4B04D447 \
D20D85FD A50AB56B 35B5A8FA 42B2986C DBBBC9D6 ACBCF940 32D86CE3 45DF5C75 \
DCD60DCF ABD13D59 26D930AC 51DE003A C8D75180 BFD06116 21B4F4B5 56B3C423 \
CFBA9599 B8BDA50F 2802B89E 5F058808 C60CD9B2 B10BE924 2F6F7C87 58684C11 \
C1611DAB B6662D3D 76DC4190 01DB7106 98D220BC EFD5102A 71B18589 06B6B51F \
9FBFE4A5 E8B8D433 7807C9A2 0F00F934 9609A88E E10E9818 7F6A0DBB 086D3D2D \
91646C97 E6635C01 6B6B51F4 1C6C6162 856530D8 F262004E 6C0695ED 1B01A57B \
8208F4C1 F50FC457 65B0D9C6 12B7E950 8BBEB8EA FCB9887C 62DD1DDF 15DA2D49 \
8CD37CF3 FBD44C65 4DB26158 3AB551CE A3BC0074 D4BB30E2 4ADFA541 3DD895D7 \
A4D1C46D D3D6F4FB 4369E96A 346ED9FC AD678846 DA60B8D0 44042D73 33031DE5 \
AA0A4C5F DD0D7CC9 5005713C 270241AA BE0B1010 C90C2086 5768B525 206F85B3 \
B966D409 CE61E49F 5EDEF90E 29D9C998 B0D09822 C7D7A8B4 59B33D17 2EB40D81 \
B7BD5C3B C0BA6CAD EDB88320 9ABFB3B6 03B6E20C 74B1D29A EAD54739 9DD277AF \
04DB2615 73DC1683 E3630B12 94643B84 0D6D6A3E 7A6A5AA8 E40ECF0B 9309FF9D \
0A00AE27 7D079EB1 F00F9344 8708A3D2 1E01F268 6906C2FE F762575D 806567CB \
196C3671 6E6B06E7 FED41B76 89D32BE0 10DA7A5A 67DD4ACC F9B9DF6F 8EBEEFF9 \
17B7BE43 60B08ED5 D6D6A3E8 A1D1937E 38D8C2C4 4FDFF252 D1BB67F1 A6BC5767 \
3FB506DD 48B2364B D80D2BDA AF0A1B4C 36034AF6 41047A60 DF60EFC3 A867DF55 \
316E8EEF 4669BE79 CB61B38C BC66831A 256FD2A0 5268E236 CC0C7795 BB0B4703 \
220216B9 5505262F C5BA3BBE B2BD0B28 2BB45A92 5CB36A04 C2D7FFA7 B5D0CF31 \
2CD99E8B 5BDEAE1D 9B64C2B0 EC63F226 756AA39C 026D930A 9C0906A9 EB0E363F \
72076785 05005713 95BF4A82 E2B87A14 7BB12BAE 0CB61B38 92D28E9B E5D5BE0D \
7CDCEFB7 0BDBDF21 86D3D2D4 F1D4E242 68DDB3F8 1FDA836E 81BE16CD F6B9265B \
6FB077E1 18B74777 88085AE6 FF0F6A70 66063BCA 11010B5C 8F659EFF F862AE69 \
616BFFD3 166CCF45 A00AE278 D70DD2EE 4E048354 3903B3C2 A7672661 D06016F7 \
4969474D 3E6E77DB AED16A4A D9D65ADC 40DF0B66 37D83BF0 A9BCAE53 DEBB9EC5 \
47B2CF7F 30B5FFE9 BDBDF21C CABAC28A 53B39330 24B4A3A6 BAD03605 CDD70693 \
54DE5729 23D967BF B3667A2E C4614AB8 5D681B02 2A6F2B94 B40BBE37 C30C8EA1 5A05DF1B 2D02EF8D';

    for (; i < length; i++)
      crc = (crc >>> 8) ^ ('0x' + map.substr(((crc ^ str.charCodeAt(i)) & 0xFF) * 9, 8));

    crc = crc ^ (-1);
    return (crc < 0 ? crc += 4294967296 : crc)
      .toString()
      .split('')
      .map(function (v) { return v.charCodeAt(0) });
  }

});


