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

import * as fs from 'somes/fs';
import * as marked from 'marked';
var renderer = new marked.Renderer();

var marked_template = fs.readFileSync(__dirname + '/../marked/template.html').toString('utf-8');

function get_marked_template() {
	return marked_template;
}

renderer.heading = function(text, level, raw) {
	var id = raw.toLowerCase().replace(/\`+/g, '').replace(/[^\w]+/g, '-');
	var toc = this.m_toc;

	if ( toc ) {
		if ( this.m_prev_level < level ) { // into
			toc.push('<ul>');
			this.m_prev_level++;
		} 
		else if ( this.m_prev_level > level ) { // out
			toc.push('</li>');
			toc.push('</ul>');
			this.m_prev_level--;
		} 
		else {
			toc.push('</li>');
		}
		toc.push('<li>');
		toc.push(`<span><a href="#${id}">${raw.replace(/\`+/g, '')}</a></span>`);
	}

	return (
		`<h${level} id="${id}">
			${text}
			<span><a class="mark" href="#${id}" id="${id}">#</a></span>
		</h${level}>`
	);
};

export function gen_html(text_md, title, template) {
	var lexer = new marked.Lexer();
	var tokens = lexer.lex(text_md);

	renderer.m_toc = [ ];
	renderer.m_prev_level = 0;
	template = template || get_marked_template();

	var body = marked.parser(tokens, { renderer: renderer });

	for ( var i = renderer.m_prev_level; i > 0; i-- ) {
		renderer.m_toc.push('</li>');
		renderer.m_toc.push('</ul>');
	}
	template = template.replace(/__placeholder_toc__/g, renderer.m_toc.join('\n'));
	renderer.m_toc = null;
	renderer.m_prev_level = 0;

	template = template.replace(/__placeholder_title__/g, title || '');
	template = template.replace(/__placeholder_body__/g, body);

	return { html: template, tokens: tokens };
}

export const get_default_template = get_marked_template;