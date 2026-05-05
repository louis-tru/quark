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

const { exec } = require('qktool/node/syscall');
const argument = require('qktool/arguments');
const fs      = require('fs');
const path    = require('path');
const {check_file_is_change} = require('./check');
const inputs  = process.argv.slice(2)
	.filter(e=>e.substring(0,2) != '--'); // delete option, for example: --watch
const output_cc = inputs.pop();
const output_h = inputs.pop();
const glslc = argument.options.glslc || 'glslc';
const spirv_cross = argument.options.spirv_cross || 'spirv-cross';

const round = `
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

async function exec2(cmd) {
	var r = await exec(cmd, {
		stdout: process.stdout,
		stderr: process.stderr, stdin: process.stdin,
	});
	if (r.code != 0) {
		throw Error.new(`Run cmd fail, "${cmd}", code = ${r.code}`);
	}
}

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
		.replace(/^\s+/mg, '') // delete indent
		.replace(/\s*\/\/(?!\!\<).*$/mg, '') // delete comment but not delete //!< comment
		.replace(/\/\*.*?\*\//mg, '') // delete /* */ comment
		// .replace(/\\/mg, '\\\\'); // escape \ to \\, because we will put the code into c++ string, and \ is escape char in c++
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

const all_import_sources = {};

// find uniform and attribute
// var reg = /^\s*(?:layout\s*\(\s*location\s*=\s*(\d+)\s*\)\s+)?
// (uniform|attribute|in)\s+((lowp|mediump|highp)\s+)?
// (int|float|vec2|vec3|vec4|mat2|mat3|mat4|sampler2D)
// \s+([a-zA-Z0-9\_]+)\s*(\[\s*(\d+)\s*\])?;\s*$/mg;
const find_regexp_pre = new RegExp(
	'^\\s*(?:layout\\s*\\(\\s*(location|binding)\\s*=\\s*(\\d+)\\s*\\)\\s+)?'+
	'(uniform|attribute|in)\\s+((lowp|mediump|highp)\\s+)?'+
	'(float|vec2|vec3|vec4|int|ivec2|ivec3|ivec4|uint|uvec2|uvec3|uvec4|mat2|mat3|mat4|sampler2D)'+
	'\\s+([a-zA-Z0-9\\_]+)\\s*(\\[\\s*(\\d+)\\s*\\])?;\\s*(?:\\/\\/\\!\\<\\s*\\{(.+?)\\}.*)?$'
	,'mg'
);

function parse_type_info(type, name, arr, location, glTMap) {
	let info = glTypeSizeInformation[type];
	if (!info)
		return {
			name,
			type,
			...(location ? {location}: {binding: location})
		};
	let [sizeT,glT,ccType] = info;
	let arrN = arr ? Number(arr): 1;
	let size = sizeT*arrN;
	// if exist glTMap, means the attribute is normalized, and the real gl type is GL_UNSIGNED_BYTE or GL_BYTE or ...
	// for example: in vec4 lightColorIn; //!< {GL_UNSIGNED_BYTE} 4 bytes, RGBA
	// let glTMap = mat[10]; //!< {GL_UNSIGNED_BYTE} or //!< {GL_BYTE} or ...
	let normalized = 'GL_FALSE';

	if (glTMap && glTMap != glT) {
		glT = glTMap;
		ccType = glTypeMapCpp[glT];
		normalized = 'GL_TRUE';
	}
	return {
		name,
		size: size,
		type: type,
		glType: glT,
		ccType: ccType,
		stride: `sizeof(${ccType})*${size}`,
		arr: arrN>1?arrN:0,
		normalized,
		...(location ? {location}: {binding: location})
	};
}

function find_uniforms_attributes_pre(code, uniforms, attributes) {
	find_regexp_pre.lastIndex = 0; // reset index for global regexp
	var mat = find_regexp_pre.exec(code);

	while (mat) {
		let locationOrBinding = mat[2];
		let name = mat[7];
		let type = mat[6];
		let arr = mat[9];
		if (mat[3] == 'uniform') {
			if (type == 'sampler2D') { // ignore non sampler2D uniform
				uniforms.push({
					type,
					name,
					glType: 'GL_SAMPLER_2D',
					nameSlot: name + 'Slot', // for example: textureSlot
					binding: locationOrBinding,
				});
			}
		// } else if (name.substring(name.length - 2) == 'In') { // attribute | in
		} else if (mat[3] == 'in') { // in
			// if exist glTMap, means the attribute is normalized, and the real gl type is GL_UNSIGNED_BYTE or GL_BYTE or ...
			// for example: in vec4 lightColorIn; //!< {GL_UNSIGNED_BYTE} 4 bytes, RGBA
			let glTMap = mat[10]; //!< {GL_UNSIGNED_BYTE} or //!< {GL_BYTE} or ...
			attributes.push(
				parse_type_info(type, name, arr, locationOrBinding, glTMap)
			);
		}

		mat = find_regexp_pre.exec(code);
	}
}

const find_regexp = new RegExp(
	'^\\s*(layout\\s*\\(\\s*binding\\s*=\\s*(\\d+)\\s*,\\s*std(\\d+)\\s*\\)\\s+)?'+
	'(uniform|struct)\\s+'+
	'([a-zA-z0-9\\_\\$]+)'+ // type or struct name
	'('+
	'\\s+([a-zA-Z0-9\\_\\$]+)\\s*;|\\s*\\{'+ // name; or { for struct or uniform block
	'([^\\}]+)\\}' +
	')'
	,'mg'
);
function find_uniforms_attributes(code, uniforms, uniform_blocks, structs) {
	find_regexp.lastIndex = 0; // reset index for global regexp
	let mat = find_regexp.exec(code);

	function parse_block(blockStr) {
		// struct PcArgs
		// {
		// 		mediump float depth;
		// 		mediump float allScale;
		// 		mediump vec4 texCoords;
		// 		mediump vec4 color;
		// };
		const block = [];
		const reg = /^\s*((mediump|lowp|highp)\s+)?([a-zA-Z0-9\_\$]+)\s+([a-zA-Z0-9\_]+)\s*(\[\s*(\d+)\s*\])?;\s*$/mg;
		let mat = reg.exec(blockStr);

		while (mat) {
			const type = mat[3];
			const name = mat[4];
			const arr = mat[6]; // vec4 colors[256]; arr = 256
			block.push(
				parse_type_info(type, name, arr, '', '')
			);
			mat = reg.exec(blockStr);
		}
		return block;
	}

	while (mat) {
		let binding = mat[2];
		let std = mat[3];
		let key = mat[4]; // uniform or struct
		let type = mat[5]; // name for uniform, struct type name for struct
		let block = mat[8];
		if (key == 'uniform') { // uniform block or uniform
			if (block) { // uniform block
				uniform_blocks.push({
					type, binding, std, block: parse_block(block),
				});
			} else if (type != 'sampler2D') { // ignore sampler2D uniform,
				// because we have already get sampler2D uniform in find_uniforms_attributes_pre
				// uniform, for example: uniform vec4 color; or uniform mat4 transform;
				// or uniform PcArgs pc; (PcArgs is struct type)
				const name = mat[7];
				uniforms.push({
					type,
					name,
					binding, std,
					...parse_type_info(type, name, '', '', ''),
				});
			}
		} else if (key == 'struct') { // struct
			if (block) {
				structs.push({ type, block: parse_block(block) });
			}
		}

		mat = find_regexp.exec(code);
	}
}

function resolve_source_part(dirname, codestr) {
	let source_list = [];
	let reg = /^#import\s+"([^"]+)"/gm;
	let lastIndex = 0;
	let mat;

	while (mat = reg.exec(codestr)) {
		if (lastIndex != mat.index) {
			source_list.push(codestr.substring(lastIndex, mat.index));
		}
		let imp = path.resolve(dirname, mat[1]);
		let s = resolve_source_both('', imp);
		source_list.push(s); // add source to sources
		lastIndex = mat.index + mat[0].length;
	}

	if (lastIndex!= codestr.length) {
		source_list.push(codestr.substring(lastIndex));
	}
	return source_list;
}

function resolve_source_both(beforeCode, filename) {
	let pathname = path.resolve(filename);
	let source = all_import_sources[pathname];
	if (source) {
		return source;
	}
	console.log(`gen-glsl ${pathname}`);

	let result = {filename};
	let dirname = path.dirname(pathname);
	let codestr = readcode(pathname);

	let [first, fragstr] = codestr.split(/^#frag/gm); // split vert and frag
	let [util, vertstr] = first.split(/^#vert/gm); // split util and vert


	vertstr = beforeCode + util + (vertstr || ''); // util + vert
	fragstr = beforeCode + util + (fragstr || ''); // util + frag

	all_import_sources[pathname] = result; // cache source, for example: {filename, vert, frag}

	result['vert'] = resolve_source_part(dirname, vertstr);
	result['frag'] = resolve_source_part(dirname, fragstr);

	return result;
}

function marge_source(vert, source, output, import_set) {
	for (let part of source) {
		if (typeof part == 'string') {
			output.push(part);
		} else if (!import_set.has(part.filename)) { // if not import, marge source, else ignore
			import_set.add(part.filename);
			marge_source(vert, vert ? part.vert: part.frag, output, import_set);
		}
	}
}

async function resolve_ast(name, stage, source_both) {
	const source_arr = [];
	marge_source(stage=='vert', source_both[stage], source_arr, new Set());

	let source = source_arr.join('');
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
	let uniform_blocks = [];
	let structs = [];

	let if_reg = / Qk_SHADER_IF_FLAGS_([a-z0-9\_]+)/igm,if_m;
	// query if flags
	while (if_m = if_reg.exec(source)) {
		if_flags.push(if_m[1]);
	}

	const glsl_out = `${__dirname}/../src/render/shader/out/glsl/${name}.${stage}.glsl`;
	const spv_out = `${__dirname}/../src/render/shader/out/spv/${name}.${stage}.spv`;
	const es450_out = `${__dirname}/../src/render/shader/out/es450/${name}.${stage}.es450.glsl`;
	const es300_out = `${__dirname}/../src/render/shader/out/es300/${name}.${stage}.es300.glsl`;
	const metal_out = `${__dirname}/../src/render/shader/out/metal/${name}.${stage}.metal`;

	fs.writeFileSync(glsl_out, source, 'utf8');

	await exec2(`${glslc} -fshader-stage=${stage} ${glsl_out} -o ${spv_out}`);
	await exec2(`${spirv_cross} ${spv_out} --es --version 300 > ${es300_out}`);
	await exec2(`${spirv_cross} ${spv_out} --es --version 450 > ${es450_out}`);
	await exec2(`${spirv_cross} ${spv_out} --msl --rename-entry-point main main0 ${stage} --stage ${stage} > ${metal_out}`);

	const source_es300 = fs.readFileSync(es300_out).toString('utf8');
	const source_es450 = fs.readFileSync(es450_out).toString('utf8');

	find_uniforms_attributes_pre(source, uniforms, attributes);
	find_uniforms_attributes(source_es450, uniforms, uniform_blocks, structs);

	for (let uniform of uniforms) {
		if (!uniform.glType) {
			const type_struct = structs.find(s=>s.type == uniform.type);
			if (type_struct) {
				uniform.glType = 'struct ' + uniform.type;
				uniform.struct = type_struct;
			}
		}
	}

	let ast = {
		name,
		stage,
		source,
		source_es300,
		attributes,
		uniforms,
		uniform_blocks,
		structs,
		if_flags,
	};
	return ast;
}

async function resolve_doc(name_, input) {
	const source_both = resolve_source_both('#import "_util.glsl"\n', input);
	const name = name_.replace(/[\-_](.)/gm, (_,b)=>b.toUpperCase());
	const className = `${name[0].toUpperCase()}${name.substring(1)}`;
	const vert_ast = await resolve_ast(name, 'vert', source_both);
	const frag_ast = await resolve_ast(name, 'frag', source_both);

	let set = {};

	let uniform_blocks = vert_ast.uniform_blocks.concat(frag_ast.uniform_blocks)
		.filter(e=>(set[e.type+'_block'] ? 0: (set[e.type+'_block']=1,1)));

	// sort uniform blocks by binding index
	uniform_blocks.sort((a,b)=>Number(a.binding)-Number(b.binding));

	let structs = vert_ast.structs.concat(frag_ast.structs)
		.filter(e=>(set[e.type+'_struct'] ? 0: (set[e.type+'_struct']=1,1)));

	let uniforms = vert_ast.uniforms.concat(frag_ast.uniforms)
		.filter(e=>(set[e.name] ? 0: (set[e.name]=1,1)));

	uniforms.sort((a,b)=>Number(a.binding || 9999) - Number(b.binding || 9999));

	let uniforms_commom = uniforms.filter(e=>!e.struct);
	let uniforms_struct = uniforms.filter(e=>e.struct);
	let uniforms_sampler2D = uniforms.filter(e=>e.glType == 'GL_SAMPLER_2D');

	let if_flags = vert_ast.if_flags.concat(frag_ast.if_flags)
		.reduce((a,i)=>((a.indexOf(i)==-1?a.push(i):void 0),a), []);

	return { // return doc
		name, // for example: colorRadial
		className, // for example: ColorRadial
		source_both,
		vert_ast, //
		frag_ast, //
		attributes: vert_ast.attributes, // doc vert attributes
		uniform_blocks, // doc all uniform blocks
		structs, // doc all structs
		uniforms, // doc all uniforms
		uniforms_commom, // doc all common uniforms, means not struct uniforms
		uniforms_struct, // doc all struct uniforms
		uniforms_sampler2D, // doc all sampler2D uniforms
		if_flags, // doc all if flags
		glal_native_get_call: '',
	};
}

// generate glsl native code to cpp and hpp
function gen_glsl_native_code(glslDocs, output_h, output_cc) {
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

	function write_cpp(ast, cpp) {
		let get_call = `get_${ast.name}()`;
		let code = ast.source_es300
			.replace(/#version\s+\d+(\s*[a-z]+)\n?/mg, '') // delete version
			.replace(/^\s+/mg, '') // delete indent
			.replace(/\\/mg, '\\\\'); // escape \ to \\, because we will put the code into c++ string, and \ is escape char in c++
		let codeLen = Buffer.byteLength(code, 'utf8');

		ast.glal_native_get_call = `get_glsl_${ast.name}_${ast.stage}()`; // for example: get_glsl_color()

		write(cpp, `	String ${ast.glal_native_get_call} {`,
			`		const char* c = "${code.replace(/\n/gm, '\\n\\\n')}";`,
			`		return String(c, ${codeLen});`,
		'	}',
		);
	}

	for (let doc of glslDocs) {
		write_cpp(doc.vert_ast, cpp);
		write_cpp(doc.frag_ast, cpp);

		const {uniforms_commom,uniforms_struct,uniforms_sampler2D} = doc;

		// write hpp
		write(hpp, `	struct GLSL${doc.className}: GLSLShader {`,
			doc.structs.length ? doc.structs.map(s=>[
				`		struct ${s.type} {`,
					s.block.map(b=>
						`			${b.ccType} ${b.name}${b.size>1?`[${b.size}]`:''}${b.arr?`[${b.arr}]`:''}; // ${b.type}${b.arr?`[${b.arr}]`:''} ${b.name}`),
				`		};`
			]) : '',
			doc.attributes.length ? `		GLuint ${doc.attributes.map(e=>e.name).join(',')}; // attributes location`: '',
			uniforms_commom.length ? `		GLuint ${uniforms_commom.map(e=>e.name).join(',')}; // uniforms location`: '',
			uniforms_struct.length ? uniforms_struct.map(e=>`		GLuint ${e.struct.block.map(it=>`${e.name}_${it.name}`).join(',')}; // struct uniform block location`) : '',
			uniforms_sampler2D.length ? `		GLuint ${uniforms_sampler2D.map(e=>e.nameSlot).join(',')}; // sampler2D texture slot`: '',
			doc.uniform_blocks.length ? `		GLuint ${doc.uniform_blocks.map(e=>e.type).join(',')}; // uniform block binding index`: '',
			`		virtual void build(const char* name, const char *macros);`,
		`	};`);

		// write cpp
		write(cpp, `void GLSL${doc.className}::build(const char* name, const char * macros) {`,
			`	gl_compile_link_shader(this, name,macros,`,
				`	${doc.vert_ast.glal_native_get_call},${doc.frag_ast.glal_native_get_call},`,
			'	{',
					doc.attributes.map(e=>`		{"${e.name}",${e.size},${e.glType},${e.stride},${e.normalized},&${e.name}},`),
			'	},',
			'	{',
					uniforms_commom.map(e=>`		{"${e.name}",${e.glType},&${e.name},${e.nameSlot?'&'+e.nameSlot:0}},`),
					uniforms_struct.map(e=>e.struct.block.map(it=>`		{"${e.name}.${it.name}",${it.glType},&${e.name}_${it.name},0},`)),
			'	},',
			'	{',
					doc.uniform_blocks.map(e=>`		{"${e.type}",&${e.type}},`),
			'	});',
			'}'
		);
	}

	write(hpp, '	struct GLSLShaders {');
	write(cpp, 'void GLSLShaders::buildAll() {');

	for (let doc of glslDocs) {
		write(cpp, `	${doc.name}.build("${doc.name}", "");`);
		write(hpp, `		GLSL${doc.className} ${doc.name};`);
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

async function main(output_h, output_cc) {
	if ( !check_file_is_change(inputs.concat([__filename]), [output_h, output_cc]) )
		return;

	console.log(process.cwd(), output_h, output_cc);

	for (let dir of ['glsl', 'es300', 'es450', 'spv', 'metal']) {
		fs.mkdirSync(`${__dirname}/../src/render/shader/out/${dir}`, { recursive: true }); // make out dir if not exist
	}

	const glslDocs = [];

	for (let input of inputs) {
		const mat = input.match(/[\/\\]([a-z][^\/\\]+)\.glsl$/i);
		if ( mat ) {
			const name = mat[1];
			glslDocs.push(await resolve_doc(name, input));
		}
	}

	gen_glsl_native_code(glslDocs, output_h, output_cc);
}

main(output_h, output_cc);

// test
// main(`${__dirname}/../src/render/gl/glsl_shaders.h`, `${__dirname}/../src/render/gl/glsl_shaders.cc`);