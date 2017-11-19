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

var util = require('./util');

function split_path(self) {
  if (self._is_split) return;
  self._is_split = true;
  
  var value = self._value;
  var value2 = '';
  var i = value.indexOf('?');
  
  if (i != -1) {
    value2 = value.substr(0, i);
    var search = self._search = value.substr(i);
    i = search.indexOf('#');
    if (i != -1) {
      self._search = search.substr(0, i);
      self._hash = search.substr(i);
    }
  } else {
    i = value.indexOf('#');
    if (i != -1) {
      value2 = value.substr(0, i);
      self._hash = value.substr(i);
    } else {
      value2 = value;
    }
  }
  self._value = util.format(value2);
  
  if (value2[value2.length - 1] == '/')
    self._suffix = '/';
}

function parse_base_ext_name(self) {
  if (self._base == -1) {
    split_path(self);
    var mat = self._value.match(/([^\/\\]+)?(\.[^\/\\\.]+)$|[^\/\\]+$/);
    if (mat) {
      self._base = mat[0];
      self._ext = mat[2] || '';
    } else {
      self._ext = self._base = '';
    }
  }
}

function parse_path(self) {
  if (self._is_parse) return;
  self._is_parse = true;
  
  split_path(self);
  
  var value = self._value;
  var label = '';
  var path = '';
  var mat = value.match(
    /^((\/)|(([a-z]+:)\/\/(([^\/:]+)(:\d+)?))|(file:\/\/\/([a-z]:(?![^\/]))?)|([a-z]:(?![^\/])))/i);
  
  if (mat) {
    if (mat[2]) { // unix absolute path
      self._protocol = 'file:';
      self._origin = 'file://';
      path = value;
    } else if (mat[3]) { // network protocol
      self._origin = mat[3];
      self._protocol = mat[4];
      self._host = mat[5];
      self._hostname = mat[6];
      if (mat[7])  // port
        self._port = mat[7].substr(1);
      path = value.substr(self._origin.length);
    } else if (mat[8]) { // local file protocol
      self._protocol = 'file:';
      self._origin = 'file://';
      if (mat[9]) { // windows path and volume label
        self._prefix = '/';
        label = mat[9].toUpperCase();
        path = value.substr(10);
      } else { // unix path
        path = value.substr(7);
      }
    } else { //  windows local file
      self._protocol = 'file:';
      self._origin = 'file://';
      self._prefix = '/';
      label = value.substr(0, 2).toUpperCase();
      path = value.substr(2);
    }
  }
  
  self._label = label;
  
  if (!path) {
    self._dir = label + '/';
    self._path = label + '/';
  } else {
    var i = path.lastIndexOf('/');
    if (i > 0) 
      self._dir = label + path.substr(0, i);
    else
      self._dir = label + '/';
    self._path = label + path;
  }
  
  if (self._path.length == 1) 
    self._suffix = '';
}

function parse_params(self) {
  if (self._params) 
    return;
  split_path(self);
  
  var params = self._params = { };
  
  if (self._search[0] != '?') 
    return;
    
  var ls = self._search.substr(1).split('&');
  
  for (var i = 0; i < ls.length; i++) {
    var o = ls[i].split('=');
    params[ o[0] ] = o[1] || '';
  }
}

function parse_hash_params(self) {
  if (self._hash_params) 
    return;
  split_path(self);
  
  var params = self._hash_params = { };
  if (self._hash[0] != '#') 
    return;
    
  var ls = self._hash.substr(1).split('&');
  
  for (var i = 0; i < ls.length; i++) {
    var o = ls[i].split('=');
    params[ o[0] ] = o[1] || '';
  }
}

function stringify_params(prefix, params) {
  var rev = [];
  for (var i in params) {
    rev.push(i + '=' + params[i]);
  }
  return rev.length ? prefix + rev.join('&') : '';
}

/**
  * @class URI
  */
var URI = util.class('URI', {
  // 
  _is_split: false,
  _is_parse: false,
  _value: '',
  _host: '',
  _hostname: '',
  _port: '',
  _protocol: '',
  _search: '',
  _hash: '',
  _origin: '',
  _path: '',
  _dir: '',
  _base: -1,
  _ext: -1,
  _params: null,
  _hash_params: null,
  _prefix: '',
  _suffix: '',
  _label: '',
  
  /**
    * @param {String} path (Optional)
    * @constructor
    */
  constructor: function (path) {
    if (path) 
      this._value = path;
  },
  
  // href: "http://xxxx.xxx:81/v1.1.0/truth/path.js?sasasas&asasasa#sdsdsd"
  get href() {
    parse_path(this);
    return  this._origin + 
            this._prefix + 
            this._path + 
            this._suffix + 
            this._search + 
            this._hash;
  },
  
  /**
    * full path
    * parhname: "/D:/Documents/test.js"
    */
  get pathname() {
    parse_path(this);
    return  this._prefix + this._path + this._suffix;
  },
  
  /**
    * full path dir
    * dirname: "/D:/Documents"
    */
  get dirname() {
    parse_path(this);
    return this._prefix + this._dir;
  },
  
  /** 
    * path: "D:/Documents/test.js"
    */
  get path() {
    parse_path(this);
    return this._path;
  },
  
  /**
   * path dirname
   * dir: "D:/Documents"
   */
  get dir() {
    parse_path(this);
    return this._dir;
  },
  
  // search: "?sasasas&asasasa"
  get search() {
    split_path(this);
    return this._search;
  },
  
  // hash: "#sdsdsd"
  get hash() {
    split_path(this);
    return this._hash;
  },
  
  // host: "mooogame.com:81"
  get host() {
    parse_path(this);
    return this._host;
  },
  
  // hostname: "mooogame.com"
  get hostname() {
    parse_path(this);
    return this._hostname;
  },
  
  // origin: "http://mooogame.com:81"
  get origin() {
    parse_path(this);
    return this._origin;
  },

  // get path base name 
  get basename() {
    parse_base_ext_name(this);
    return this._base;
  },
  
  // path extname
  get extname() {
    parse_base_ext_name(this);
    return this._ext;
  },
  
  // port: "81"
  get port() {
    parse_path(this);
    return this._port;
  },
  
  // protocol: "http:"
  get protocol() {
    parse_path(this);
    return this._protocol;
  },
  
  // windows volume label
  get label() {
    parse_path(this);
    return this._label;
  },
  
  get params() {
    parse_params(this);
    return this._params;
  },
  
  get hash_params() {
    parse_hash_params(this);
    return this._hash_params;
  },
  
  // get path param
  get: function (name) {
    return this.params[name];
  },
  
  // set path param
  set: function (name, value) {
    this.params[name] = value || '';
    this._search = stringify_params('?', this._params);
    return this;
  },
  
  // del path param
  del: function (name) {
    delete this.params[name];
    this._search = stringify_params('?', this._params);
    return this;
  },
  
  // del all prams
  del_all: function (){
    this._params = { };
    this._search = '';
  },
  
  // get hash param
  get_hash: function (name) {
    return this.hash_params[name];
  },
  
  // set hash param
  set_hash: function (name, value) {
    this.hash_params[name] = value || '';
    this._hash = stringify_params('#', this._hash_params);
    return this;
  },
  
  // del hash param
  del_hash: function (name) {
    delete this.hash_params[name];
    this._hash = stringify_params('#', this._hash_params);
    return this;
  },
  
  // del hash all params
  del_hash_all: function (){
    this._hash_params = { };
    this._hash = '';
  },
  
  // relative path
  relative: function (target) {
    target = new URI(target);
    if (this.origin != target.origin || this._label != target._label)
      return  target._protocol == 'file:' ? 
              target._path : 
              target._origin + 
              target._prefix + 
              target._path;
              // 
    var ls  = this._path == '/' ? [] : this._path.split('/');
    var ls2 = target._path == '/' ? [] : target._path.split('/');
    var len = Math.max(ls.length, ls2.length);
    
    for (var i = 1; i < len; i++) {
      if (ls[i] != ls2[i]) {
        len = ls.length - i;
        if (len > 0) {
          for (var j = 0, ls = []; j < len; j++)
            ls.push('..');
          return ls.join('/') + '/' + ls2.splice(i).join('/');
        }
        return ls2.splice(i).join('/');
      }
    }
    return '.';
  },
  // @end
});

function get_path(path) {
  if (path)
    return new URI(path);
  return new URI(process.cwd());
}

module.exports = {

  URI: URI,
  
  search: function (path) {
    return get_path(path).search;
  },
  
  hash: function (path) {
    return get_path(path).hash;
  },
  
  /**
   * full path
   */
  pathname: function (path) {
    return get_path(path).pathname;
  },
  
  // full path dir 
  dirname: function (path) {
    return get_path(path).dirname;
  },
  
  path: function (path) {
    return get_path(path).path;
  },
  
  /**
   * path dir
   */
  dir: function (path) {
    return get_path(path).dir;
  },
  
  host: function (path) {
    return get_path(path).host;
  },
  
  hostname: function (path) {
    return get_path(path).hostname;
  },
  
  // href origin
  origin: function (path) {
    return get_path(path).origin;
  },
  
  // base name
  basename: function (path) {
    return get_path(path).basename;
  },
  
  // path extname
  extname: function (path) {
    return get_path(path).extname;
  },
  
  // port: "81"
  port: function (path) {
    return get_path(path).port;
  },
  
  // protocol: "http:"
  protocol: function (path) {
    return get_path(path).protocol;
  },
  
  // windows volume label
  label: function (path) {
    return get_path(path).label;
  },
  
  // href params
  params: function (path) {
    return get_path(path).params;
  },
  
  // hash params 
  hash_params: function (path) {
    return get_path(path).hash_params;
  },
  
  // get path param
  get: function (name, path) {
    return get_path(path).get(name);
  },
  
  // set path param
  set: function (name, value, path) {
    return get_path(path).set(name, value).href;
  },
  
  // del path param
  del: function (name, path) {
    return get_path(path).del(name).href;
  },
  
  // del all hash param
  del_all: function (path) {
    return get_path(path).del_all().href;
  },
  
  // get hash param
  get_hash: function (name, path) {
    return get_path(path).get_hash(name);
  },
  
  // set hash param
  set_hash: function (name, value, path) {
    return get_path(path).set_hash(name, value).href;
  },
  
  // del hash param
  del_hash: function (name, path) {
    return get_path(path).del_hash(name).href;
  },
  
  // del all hash param
  del_hash_all: function (path) {
    return get_path(path).del_hash_all().href;
  },
  
  // relative path
  relative: function (path, target) {
    if (arguments.length > 1) 
      return get_path(path).relative(target);
    else 
      return get_path().relative(path);
  },
  
  // is absolute path
  is_absolute: util.is_absolute,
  
  // format resolve path 
  format: util.format,
  
};
