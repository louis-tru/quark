/* ***** BEGIN LICENSE BLOCK *****
 * Distributed under the BSD license:
 *
 * Copyright (c) 2015-2016, blue.chu
 * All rights reserved.
 * 
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *     * Redistributions of source code must retain the above copyright
 *       notice, this list of conditions and the following disclaimer.
 *     * Redistributions in binary form must reproduce the above copyright
 *       notice, this list of conditions and the following disclaimer in the
 *       documentation and/or other materials provided with the distribution.
 *     * Neither the name of blue.chu nor the
 *       names of its contributors may be used to endorse or promote products
 *       derived from this software without specific prior written permission.
 * 
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL blue.chu BE LIABLE FOR ANY
 * DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 * ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 * 
 * ***** END LICENSE BLOCK ***** */

var fs      = require('fs');
var path    = require('path');
var inputs  = process.argv.slice(2);
var output_cc = inputs.pop();
var output_h = inputs.pop();
var check_file_is_change = require('./check').check_file_is_change;

var round = `
float qk_round(float num) {
	float r = floor(num);
	if ( num - r >= 0.5 ) {
		return r + 1.0;
	} else {
		return r;
	}
}
vec2 qk_round(vec2 num) {
	return vec2(qk_round(num.x), qk_round(num.y));
}
`;

function write(fp) {
	for (var i = 1; i < arguments.length; i++) {
		let arg = arguments[i];
		if (Array.isArray(arg)) {
			write(fp, ...arg);
		} else {
			if (arguments[i]) {
				fs.writeSync(fp, arguments[i], 'utf-8');
				fs.writeSync(fp, '\n', 'utf-8');
			}
		}
	}
}

function readcode(input) {
	return fs.readFileSync(input).toString('utf8')
		.replace(/\/\/.*$/mg, '')
		.replace(/\/\*.*?\*\/$/mg, '');
}

function find_uniforms_attributes(code, uniforms, uniform_blocks, attributes) {
	// find uniform and attribute
	// var reg = /^\s*(?:layout\s*\(\s*location\s*=\s*(\d+)\s*\)\s+)?
	// (uniform|attribute|in)\s+((lowp|mediump|highp)\s+)?
	// (int|float|vec2|vec3|vec4|mat2|mat3|mat4|sampler2D)
	// \s+([a-zA-Z0-9\_]+)\s*(\[\s*(\d+)\s*\])?;\s*$/mg;
	var reg = new RegExp(
		'^\\s*(?:layout\\s*\\(\\s*location\\s*=\\s*(\\d+)\\s*\\)\\s+)?'+
		'(uniform|attribute|in)\\s+((lowp|mediump|highp)\\s+)?'+
		'(int|float|vec2|vec3|vec4|mat2|mat3|mat4|sampler2D)'+
		'\\s+([a-zA-Z0-9\\_]+)\\s*(\\[\\s*(\\d+)\\s*\\])?;\\s*$'
		,'mg'
	);
	var mat = reg.exec(code);

	while ( mat ) {
		let name = mat[6];
		let type = mat[5];
		let arr = mat[8];
		if (mat[2] == 'uniform') {
			uniforms.push(name);
		} else { // attribute | in
			let typeSize = {
				int: [1,'GL_INT','int'],
				float: [1,'GL_FLOAT','float'],
				vec2: [2,'GL_FLOAT','float'],
				vec3: [3,'GL_FLOAT','float'],
				vec4: [4,'GL_FLOAT','float'],
				mat2: [4,'GL_FLOAT','float'],
				mat3: [9,'GL_FLOAT','float'],
				mat4: [16,'GL_FLOAT','float'],
				sampler2D: [16,'GL_INT','int'],
			};
			let [sizeT,glT,t] = typeSize[type];
			let arrN = arr ? Number(arr): 1;
			let size = sizeT*arrN;

			attributes.push({
				name,
				size: size,
				type: glT,
				stride: `sizeof(${t})*${size}`,
				arr: arrN>1,
			});
		}

		mat = reg.exec(code);
	}
	
	// find uniform block
	reg = /^\s*(layout\s+\(std\d+\)\s+)?uniform\s+([a-zA-Z0-9\$_]+)\s*\{/mg;
	mat = reg.exec(code);
	
	while ( mat ) { // 剔除重复的名称
		uniform_blocks.push(mat[2]);
		mat = reg.exec(code);
	}
}

const all_asts = {};

function resolve_code_ast(input, hpp, cpp) {
	let ast = all_asts[input];
	if (!ast) {
		let name = path.basename(input).replace(/[\-\.]/gm, '_');
		let dir = path.dirname(input);
		let codestr = readcode(input);
		ast = resolve_code_ast_from_codestr(name, dir, codestr, 0,0,hpp, cpp);
		all_asts[input] = ast;
	}
	return ast;
}

function get_import_all(Import, importAll, set) {
	for (let i of Import) {
		if (!set.has(i.name)) {
			set.add(i.name);
			get_import_all(i.import, importAll, set);
			importAll.push(i);
		}
	}
}

function resolve_code_ast_from_codestr(name, dirname, codestr, isVert, isFrag, hpp, cpp) {
	let Import = [];
	let import_all = [];
	let attributes = [
		// struct ShaderAttr {
		// 	const char *name;
		// 	GLint size;
		// 	GLenum type;
		//  GLsizei stride;
		// };
	];
	let uniforms = []
	var uniform_blocks = [];
	let call = `get_${name}()`; // get_color_vert_code_glsl();
	// get__util_glsl

	let source = codestr.replace(/^#import\s+"([^"]+)"/gm, function(_,a) {
		let imp = path.resolve(dirname, a);
		Import.push(resolve_code_ast(imp,hpp,cpp));
		return '';
	}).replace(/^\s+/mg, '').replace(/#version\s+300(\s+es)?/, '');

	let source_len = Buffer.byteLength(source);

	find_uniforms_attributes(source, uniforms, uniform_blocks, attributes);

	get_import_all(Import, import_all, new Set);

	// write(hpp, `const cString& ${call};`);
	if (isFrag || isVert) {
		write(cpp, `const cString& ${call} {`,
			`	static String c;`,
			`	if (c.isEmpty()) {`,
				`		c+="#version " Qk_GL_Version "\\n";`,
				isFrag ? `		c+="#define Qk_SHAFER_FRAG\\n";`:'',
				import_all.map(e=>(`		c.append(${e.call});`)),
				`		c.append("${source.replace(/\n/gm, '\\n\\\n')}",${source_len});`,
			`	}`,
			`	return c;`,
		'}',
		);
	} else {
		write(cpp, `const char* ${call} {`,
			`const char* c = "${source.replace(/\n/gm, '\\n\\\n')}";`,
			`return c;`,
		'}',
		);
	}

	if (!isFrag && !isVert) {
		call += ',' + source_len;
	}

	let ast = {
		name,
		import: Import,
		import_all,
		source: source,
		call: call,
		attributes, // []
		uniforms, // string[]
		uniform_blocks,
	};

	return ast;
}

function resolve_glsl(name, input, hpp, cpp) {
	console.log(`gen-glsl ${name}`);

	let pathname = path.resolve(input);
	let dir = path.dirname(pathname);
	let codestr = readcode(pathname);
	let [vertstr, fragstr] = codestr.split(/^#frag/gm);

	vertstr = vertstr.replace(/^#vert/gm, '');
	vertstr = '#import "_util.glsl"\n' + vertstr;
	fragstr = '#import "_util.glsl"\n' + fragstr;

	let vert_ast = resolve_code_ast_from_codestr(name+'_vert', dir, vertstr, 1, 0, hpp, cpp);
	let frag_ast = resolve_code_ast_from_codestr(name+'_ftag', dir, fragstr, 0, 1, hpp, cpp);
	let set = {};

	let attributes = vert_ast.import_all
		.reduce((a,i)=>(a.push(...i.attributes),a), []).concat(vert_ast.attributes);

	let uniforms_ = vert_ast.import_all.concat(frag_ast.import_all)
		.reduce((a,i)=>(a.push(...i.uniforms),a), []).concat(vert_ast.uniforms,frag_ast.uniforms);

	let uniforms = uniforms_.filter(e=>(set[e] ? 0: (set[e]=1,1)));
	let className = `GLSL${name[0].toUpperCase()}${name.substring(1)}`;

	write(hpp, `	struct ${className}: GLSLShader {`,
		attributes.length ? `		GLuint ${attributes.map(e=>e.name).join(',')}; // attributes`: '',
		uniforms.length ? `		GLuint ${uniforms.join(',')}; // uniforms`: '',
		`		virtual void build();`,
	`	};`);

	// { {"girth_in",1,GL_FLOAT,(void*)(sizeof(float)*2)} }

	write(cpp, `void ${className}::build() {`,
		`	gl_compile_link_shader(this, "${name}",`,
		`	${vert_ast.call},${frag_ast.call},`,
		'	{',
			attributes.map(e=>`		{"${e.name}",${e.size},${e.type},${e.stride}},`),
		'	},',
		`	"${uniforms.join(',')}");`,
		'}'
	);
}

function main() {
	if ( !check_file_is_change(inputs.concat([__filename]), [output_h, output_cc]) )
		return;

	console.log(process.cwd(), output_h, output_cc);

	var hpp = fs.openSync(output_h, 'w');
	var cpp = fs.openSync(output_cc, 'w');
	var pair_inputs = {};

	inputs.forEach(function(input) {
		var mat = input.match(/[\/\\]([a-z][^\/\\]+)\.glsl$/i);
		if ( mat ) {
			var name = mat[1];
			pair_inputs[name] = {name,input}
		}
	});

	var now = Date.now();
	write(hpp,
		'#ifndef __gl_shader_natives_' + now,
		'#define __gl_shader_natives_' + now,
		'#include "gl_shader.h"',
		'namespace qk {',
		// '#pragma pack(push,4)',
	);

	write(cpp,
		`#include "./${path.basename(output_h)}"`,
		'namespace qk {',
	);

	for (let {name,input} of Object.values(pair_inputs)) {
		name = name.replace(/[\-_](.)/gm, (_,b)=>b.toUpperCase());
		resolve_glsl(name, input, hpp, cpp);
	}

	// write(hpp, '#pragma pack(pop)');
	write(hpp, '}', '#endif'); // end
	write(cpp, '}'); // end

	fs.closeSync(hpp);
	fs.closeSync(cpp);
}

main();
