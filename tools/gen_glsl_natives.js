/* ***** BEGIN LICENSE BLOCK *****
 * Distributed under the BSD license:
 *
 * Copyright (c) 2015-2016, Louis.chu
 * All rights reserved.
 * 
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *     * Redistributions of source code must retain the above copyright
 *       notice, this list of conditions and the following disclaimer.
 *     * Redistributions in binary form must reproduce the above copyright
 *       notice, this list of conditions and the following disclaimer in the
 *       documentation and/or other materials provided with the distribution.
 *     * Neither the name of Louis.chu nor the
 *       names of its contributors may be used to endorse or promote products
 *       derived from this software without specific prior written permission.
 * 
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL Louis.chu BE LIABLE FOR ANY
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
			if (arg) {
				fs.writeSync(fp, arg, 'utf-8');
				fs.writeSync(fp, '\n', 'utf-8');
			}
		}
	}
}

function readcode(input) {
	return fs.readFileSync(input).toString('utf8')
		.replace(/\s*\/\/(?!\!\<).*$/mg, '') // delete comment but not delete //!< comment
		.replace(/\/\*.*?\*\//mg, '') // delete /* */ comment
		.replace(/\\/mg, '\\\\'); // escape \ to \\, because we will put the code into c++ string, and \ is escape char in c++
}

const glTypeSizeInformation = {
	float: [1,'GL_FLOAT','float'],
	vec2:  [2,'GL_FLOAT','float'],
	vec3:  [3,'GL_FLOAT','float'],
	vec4:  [4,'GL_FLOAT','float'],

	int:   [1,'GL_INT','int'],
	ivec2: [2,'GL_INT','int32_t'],
	ivec3: [3,'GL_INT','int32_t'],
	ivec4: [4,'GL_INT','int32_t'],

	uint:  [1,'GL_UNSIGNED_INT','uint32_t'],
	uvec2: [2,'GL_UNSIGNED_INT','uint32_t'],
	uvec3: [3,'GL_UNSIGNED_INT','uint32_t'],
	uvec4: [4,'GL_UNSIGNED_INT','uint32_t'],

	mat2:  [4,'GL_FLOAT','float'],
	mat3:  [9,'GL_FLOAT','float'],
	mat4:  [16,'GL_FLOAT','float'],
	sampler2D: [16,'GL_INT','int'],
};

const glTypeMapCpp = {
	GL_BYTE: 'int8_t',
	GL_UNSIGNED_BYTE: 'uint8_t',
	GL_SHORT: 'int16_t',
	GL_UNSIGNED_SHORT: 'uint16_t',
	GL_INT: 'int32_t',
	GL_UNSIGNED_INT: 'uint32_t',
	GL_FLOAT: 'float',
};

function find_uniforms_attributes(code, uniforms, uniform_blocks, attributes) {
	// find uniform and attribute
	// var reg = /^\s*(?:layout\s*\(\s*location\s*=\s*(\d+)\s*\)\s+)?
	// (uniform|attribute|in)\s+((lowp|mediump|highp)\s+)?
	// (int|float|vec2|vec3|vec4|mat2|mat3|mat4|sampler2D)
	// \s+([a-zA-Z0-9\_]+)\s*(\[\s*(\d+)\s*\])?;\s*$/mg;
	var reg = new RegExp(
		'^\\s*(?:layout\\s*\\(\\s*location\\s*=\\s*(\\d+)\\s*\\)\\s+)?'+
		'(uniform|attribute|in)\\s+((lowp|mediump|highp)\\s+)?'+
		'(float|vec2|vec3|vec4|int|ivec2|ivec3|ivec4|uint|uvec2|uvec3|uvec4|mat2|mat3|mat4|sampler2D)'+
		'\\s+([a-zA-Z0-9\\_]+)\\s*(\\[\\s*(\\d+)\\s*\\])?;\\s*(?:\\/\\/\\!\\<\\s*\\{(.+?)\\}.*)?$'
		,'mg'
	);
	var mat = reg.exec(code);

	while (mat) {
		let name = mat[6];
		let type = mat[5];
		let arr = mat[8];
		if (mat[2] == 'uniform') {
			uniforms.push(name);
		} else if (name.substring(name.length - 2) == 'In') { // attribute | in
			let [sizeT,glT,t] = glTypeSizeInformation[type];
			let arrN = arr ? Number(arr): 1;
			let size = sizeT*arrN;
			// if exist glTMap, means the attribute is normalized, and the real gl type is GL_UNSIGNED_BYTE or GL_BYTE or ...
			// for example: in vec4 lightColorIn; //!< {GL_UNSIGNED_BYTE} 4 bytes, RGBA
			let glTMap = mat[9]; //!< {GL_UNSIGNED_BYTE} or //!< {GL_BYTE} or ...
			let normalized = 'GL_FALSE';

			if (glTMap && glTMap != glT) {
				glT = glTMap;
				t = glTypeMapCpp[glT];
				normalized = 'GL_TRUE';
			}
			attributes.push({
				name,
				size: size,
				type: glT,
				stride: `sizeof(${t})*${size}`,
				arr: arrN>1,
				normalized,
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

function get_imports_all(imports, {if_flags,imports_all}, set) {
	for (let i of imports) {
		if (!set.has(i.name)) {
			set.add(i.name);
			get_imports_all(i.imports, {if_flags,imports_all}, set);
			imports_all.push(i);
			if_flags.push(...i.if_flags);
		}
	}
}

const all_import_asts = {};

function resolve_import_ast(imp) {
	let ast = all_import_asts[imp];
	if (!ast) {
		let name = path.basename(imp).replace(/[\-\.]/gm, '_');
		let dir = path.dirname(imp);
		let codestr = readcode(imp);
		ast = resolve_ast(name, dir, codestr);
		all_import_asts[imp] = ast;
	}
	return ast;
}

function resolve_ast(name, dirname, codestr) {
	let imports = []; // current file imports
	let imports_all = []; // all recursive imports
	let attributes = [
		// struct ShaderAttr {
		// 	const char *name;
		// 	GLint size;
		// 	GLenum type;
		//  GLsizei stride;
		//  GLboolean normalized;
		// };
	];
	let if_flags = [];
	let uniforms = [];
	var uniform_blocks = [];

	let source = codestr.replace(/^#import\s+"([^"]+)"/gm, function(_,a) {
		let imp = path.resolve(dirname, a);
		imports.push(resolve_import_ast(imp));
		return '';
	}).replace(/^\s+/mg, '').replace(/#version\s+\d+(\s+es)?/, ''); // mabey is #version 300 es

	let if_reg = / Qk_SHADER_IF_FLAGS_([a-z0-9\_]+)/igm,if_m;
	// query if flags
	while (if_m = if_reg.exec(source)) {
		if_flags.push(if_m[1]);
	}

	find_uniforms_attributes(source, uniforms, uniform_blocks, attributes);

	get_imports_all(imports, {if_flags,imports_all}, new Set());

	source = source.replace(/\s*\/\/.*$/mg, ''); // delete comment, for example: //!< {GL_UNSIGNED_BYTE} 4 bytes, RGBA

	let source_len = Buffer.byteLength(source);

	let ast = {
		name,
		imports, // current file imports ast array
		imports_all, // all recursive imports ast array
		source,
		source_len,
		attributes, // []
		uniforms, // string[]
		uniform_blocks,
		if_flags,
		is_writed_glsl_native: false, // is writed glsl native code to cpp
		glal_native_get_call: '', // get_color_vert_code_glsl(); get__util_glsl();
	};

	return ast;
}

function resolve_doc(name, filename) {
	console.log(`gen-glsl ${name}`);

	let pathname = path.resolve(filename);
	let dir = path.dirname(pathname);
	let codestr = readcode(pathname);
	let [first, fragstr] = codestr.split(/^#frag/gm); // split vert and frag
	let [util, vertstr] = first.split(/^#vert/gm); // split util and vert

	vertstr = '#import "_util.glsl"\n' + util + (vertstr || ''); // _util.glsl + util + vert
	fragstr = '#import "_util.glsl"\n' + util + (fragstr || ''); // _util.glsl + util + frag

	let vert_ast = resolve_ast(name+'_vert', dir, vertstr);
	let frag_ast = resolve_ast(name+'_frag', dir, fragstr);
	let set = {};

	let attributes = vert_ast.imports_all
		.reduce((a,i)=>(a.push(...i.attributes),a), []).concat(vert_ast.attributes);

	let uniforms_ = vert_ast.imports_all.concat(frag_ast.imports_all)
		.reduce((a,i)=>(a.push(...i.uniforms),a), []).concat(vert_ast.uniforms,frag_ast.uniforms);

	let if_flags = vert_ast.if_flags.concat(frag_ast.if_flags)
		.reduce((a,i)=>((a.indexOf(i)==-1?a.push(i):void 0),a), []);

	let uniforms = uniforms_.filter(e=>(set[e] ? 0: (set[e]=1,1)));
	let className = `${name[0].toUpperCase()}${name.substring(1)}`;

	return { // return doc
		name, // for example: colorRadial
		className, // for example: ColorRadial
		vert_ast, // doc vert ast
		frag_ast, // doc frag ast
		attributes, // doc all attributes
		uniforms, // doc all uniforms
		if_flags, // doc all if flags
	};
}

// generate glsl native code to cpp and hpp
function gen_glsl_native_code(glslDocs, output_h, output_cpp) {
	var hpp = fs.openSync(output_h, 'w');
	var cpp = fs.openSync(output_cc, 'w');
	var now = Date.now();
	write(hpp,
		'// @private head',
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

	function write_cpp(ast, isEntrance, cpp) {
		if (ast.is_writed_glsl_native)
			return;

		ast.is_writed_glsl_native = true;

		for (let imp of ast.imports) {
			write_cpp(imp, false, cpp);
		}

		ast.glal_native_get_call = `get_${ast.name}()`; // get_color_vert_code_glsl(); get__util_glsl();

		// write(hpp, `static cString& ${call};`);
		if (isEntrance) {
			write(cpp, `static cString& ${ast.glal_native_get_call} {`,
				`	static String c;`,
				`	if (c.is_empty()) {`,
						ast.imports_all.map(e=>(`		c.append(${e.glal_native_get_call});`)), // append all imports code
				`		c.append("${ast.source.replace(/\n/gm, '\\n\\\n')}",${ast.source_len});`, // append current code
				`	}`,
				`	return c;`,
			'}',
			);
		} else {
			write(cpp, `const char* ${ast.glal_native_get_call} {`,
				`const char* c = "${ast.source.replace(/\n/gm, '\\n\\\n')}";`,
				`return c;`,
			'}',
			);
			ast.glal_native_get_call += ',' + ast.source_len; // get_call + length, for example: get__util_glsl(), 1234
		}
	}

	for (let doc of glslDocs) {
		write_cpp(doc.vert_ast, true, cpp);
		write_cpp(doc.frag_ast, true, cpp);

		write(hpp, `	struct GLSL${doc.className}: GLSLShader {`,
			doc.attributes.length ? `		GLuint ${doc.attributes.map(e=>e.name).join(',')}; // attributes`: '',
			doc.uniforms.length ? `		GLuint ${doc.uniforms.join(',')}; // uniforms`: '',
			`		virtual void build(const char* name, const char *macros);`,
		`	};`);

		// { {"girth_in",1,GL_FLOAT,(void*)(sizeof(float)*2)} }

		write(cpp, `void GLSL${doc.className}::build(const char* name, const char * macros) {`,
			`	gl_compile_link_shader(this, name,macros,`,
			`	${doc.vert_ast.glal_native_get_call},${doc.frag_ast.glal_native_get_call},`,
			'	{',
				doc.attributes.map(e=>`		{"${e.name}",${e.size},${e.type},${e.stride},${e.normalized}},`),
			'	},',
			`	"${doc.uniforms.join(',')}");`,
			'}'
		);
	}

	write(hpp, '	struct GLSLShaders {');
	write(cpp, 'void GLSLShaders::buildAll() {');

	for (let glsl of glslDocs) {
		write(cpp, `	${glsl.name}.build("${glsl.name}", "");`);

		let names = [glsl.name];
		if (glsl.if_flags.length) {
			for (let if_f of glsl.if_flags) {
				let name = glsl.name + '_' + if_f;
				names.push(name);
				write(cpp, `	${glsl.name}_${if_f}.build("${name}","#define Qk_SHADER_IF_FLAGS_${if_f}\\n");`);
			}
			if (glsl.if_flags.length > 1) {
				let name = glsl.name + '_' + glsl.if_flags.join('_');
				names.push(name);
				write(cpp, `	${name}.build("${name}","${glsl.if_flags.map(e=>`#define Qk_SHADER_IF_FLAGS_${e}\\n`).join('')}");`);
			}
		}
		write(hpp, '		GLSL' + glsl.className + ' ' + names.join(',') + ';');
	}

	write(hpp,
		'		void buildAll();',
		'	};'
	);

	// write(hpp, '#pragma pack(pop)');
	write(hpp, '}', '#endif'); // end
	write(cpp, '}}'); // end

	fs.closeSync(hpp);
	fs.closeSync(cpp);
}

// generate bgfx native code
function gen_bgfx_native_code(glslDocs, output_h, output_cpp) {
	// var hpp = fs.openSync(output_h, 'w');
	// var cpp = fs.openSync(output_cc, 'w');
	// var now = Date.now();
}

function main() {
	if ( !check_file_is_change(inputs.concat([__filename]), [output_h, output_cc]) )
		return;

	console.log(process.cwd(), output_h, output_cc);

	var inputs_doc = {};
	var glslDocs = [];

	inputs.forEach(function(input) {
		var mat = input.match(/[\/\\]([a-z][^\/\\]+)\.glsl$/i);
		if ( mat ) {
			var name = mat[1];
			inputs_doc[name] = {name,input}
		}
	});

	for (let {name,input} of Object.values(inputs_doc)) {
		name = name.replace(/[\-_](.)/gm, (_,b)=>b.toUpperCase());
		glslDocs.push(resolve_doc(name, input));
	}

	gen_glsl_native_code(glslDocs, output_h, output_cc);
}

main();
