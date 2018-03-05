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

var util = require('ngui-util');
var fs = require('ngui-util/fs');
var keys = require('ngui-util/keys');
var service = require('ngui-util/service');
var HttpService = require('ngui-util/http_service').HttpService;
var server = require('ngui-tools/toolchain/server');
require('./test-multiple-service');

var opt = {
  // remote: 'http://192.168.1.124:1026/',
};

server.start_server(opt);

// Tools service
// http://127.0.0.1:1026/Tools/upload_file

var Tools = util.class('Tools', HttpService, {
  
  action: function(info) {
    HttpService.prototype.action.call(this, info);
    if ( this.form )
      this.form.is_upload = true;
  },
  
  upload_file: function() {

    console.log('params', this.params);
    console.log('data', this.data);
    
    if (this.form) {
      for (var name in this.form.files) {
        var file = this.form.files[name];
        for (var i = 0; i < file.length; i++) {
          if ( file[i].path ) {
            fs.renameSync(file[i].path, this.server.temp + '/' + file[i].filename);
          }
        }
      }
    }

    console.log('\nrequest headers:\n', this.request.headers);
    console.log('\n---------------------------', this.form ? 'form ok' : '', '\n');
    
    this.cookie.set('Hello', 'Hello', new Date(2088, 1, 1), '/');
    this.cookie.set('mark', Date.now(), new Date(2120, 1, 1), '/');
    
    this.ret_html(
      '<!doctype html>\
      <html>\
      <body>\
        <form method="post" action="/Tools/upload_file" enctype="multipart/form-data"> \
          <h2>upload file</h2>\
          <input type="file" name="upload" multiple="" />\
          <input type="submit" name="submit" value="send" />\
        </form>\
      </body>\
      </html>'
    );
  },

  test_timeout: function() {
    var self = this;
    setTimeout(function() {
      self.ret_html('OK');
    }, 5000);
  },

  test: function() {
    console.log(this.data);
    console.log(this.request.headers);
    this.response.setHeader('Access-Control-Allow-Origin', '*');
    this.ret('test_cross_domain ok');
  },

});

service.set('Tools', Tools);
