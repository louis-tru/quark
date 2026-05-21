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
const args = process.argv.slice(2)
	.filter(e=>e.substring(0,2) != '--'); // delete option, for example: --watch
const inputs = args.filter(e=>/\.glsl$/i.test(e));
const outputs = args.filter(e=>!/\.glsl$/i.test(e));
const output_gl_h = outputs.find(e=>/glsl_shaders\.h$/i.test(e));
const output_gl_cc = outputs.find(e=>/glsl_shaders\.cc$/i.test(e));
const output_mtl_h = outputs.find(e=>/mtl_shaders\.h$/i.test(e));
const output_mtl_mm = outputs.find(e=>/mtl_shaders\.mm$/i.test(e));
const output_vk_h = outputs.find(e=>/vk_shaders\.h$/i.test(e));
const output_vk_cc = outputs.find(e=>/vk_shaders\.cc$/i.test(e));
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
		.replace(/\s*\/\/(?!\!\<\s*\{.+?\}).*$/mg, '') // delete comment but not delete //!< comment
		.replace(/\/\*.*?\*\//mg, '') // delete /* */ comment
		// .replace(/\\/mg, '\\\\'); // escape \ to \\, because we will put the code into c++ string, and \ is escape char in c++
}

const informationsForTypes = {
	float: [1,'GL_FLOAT','float','MTLVertexFormatFloat'],
	vec2:  [2,'GL_FLOAT','float','MTLVertexFormatFloat2'],
	vec3:  [3,'GL_FLOAT','float','MTLVertexFormatFloat3'],
	vec4:  [4,'GL_FLOAT','float','MTLVertexFormatFloat4'],

	int:   [1,'GL_INT','int','MTLVertexFormatInt'],
	ivec2: [2,'GL_INT','int32_t','MTLVertexFormatInt2'],
	ivec3: [3,'GL_INT','int32_t','MTLVertexFormatInt3'],
	ivec4: [4,'GL_INT','int32_t','MTLVertexFormatInt4'],

	uint:  [1,'GL_UNSIGNED_INT','uint32_t','MTLVertexFormatUInt'],
	uvec2: [2,'GL_UNSIGNED_INT','uint32_t','MTLVertexFormatUInt2'],
	uvec3: [3,'GL_UNSIGNED_INT','uint32_t','MTLVertexFormatUInt3'],
	uvec4: [4,'GL_UNSIGNED_INT','uint32_t','MTLVertexFormatUInt4'],

	mat2:  [4,'GL_FLOAT','float','MTLVertexFormatFloat4'], // 2*2 = 4
	mat3:  [9,'GL_FLOAT','float','MTLVertexFormatInvalid'],
	mat4:  [16,'GL_FLOAT','float','MTLVertexFormatInvalid'],
	sampler2D: [1,'GL_INT','int','MTLVertexFormatInt'],
};

// for example: float for float, vec4 for Vec4, int for int32_t, ivec3 for IVec3, ...
const typesForCC_Vec = {
	vec2: 'Vec2',
	// std140/std430 requires vec3 to be aligned to 4 float, so add a padding float after vec3
	vec3: 'Vec3Padding',
	vec4: 'Vec4',
	ivec2: 'IVec2',
	// std140/std430 requires ivec3 to be aligned to 4 int, so add a padding int after ivec3
	ivec3: 'IVec3Padding',
	ivec4: 'IVec4',
};

const normalizedForTypes = {
	GL_BYTE: {cc: 'int8_t', msl: ['MTLVertexFormatCharNormalized',
		'MTLVertexFormatChar2Normalized', 'MTLVertexFormatChar3Normalized', 'MTLVertexFormatChar4Normalized']},
	GL_UNSIGNED_BYTE: {cc: 'uint8_t', msl: ['MTLVertexFormatUCharNormalized', 
		'MTLVertexFormatUChar2Normalized', 'MTLVertexFormatUChar3Normalized', 'MTLVertexFormatUChar4Normalized']},
	GL_SHORT: {cc: 'int16_t', msl: ['MTLVertexFormatShortNormalized', 
		'MTLVertexFormatShort2Normalized', 'MTLVertexFormatShort3Normalized', 'MTLVertexFormatShort4Normalized']},
	GL_UNSIGNED_SHORT: {cc: 'uint16_t', msl: ['MTLVertexFormatUShortNormalized', 
		'MTLVertexFormatUShort2Normalized', 'MTLVertexFormatUShort3Normalized', 'MTLVertexFormatUShort4Normalized']},
	GL_INT: {cc: 'int32_t', msl: ['MTLVertexFormatInt',
		'MTLVertexFormatInt2', 'MTLVertexFormatInt3', 'MTLVertexFormatInt4']},
	GL_UNSIGNED_INT: {cc: 'uint32_t', msl: ['MTLVertexFormatUInt',
		'MTLVertexFormatUInt2', 'MTLVertexFormatUInt3', 'MTLVertexFormatUInt4']},
	GL_FLOAT: {cc: 'float', msl: ['MTLVertexFormatFloat',
		'MTLVertexFormatFloat2', 'MTLVertexFormatFloat3', 'MTLVertexFormatFloat4']},
};

function parse_type_info(type, name, arr, location, glTNormalized) {
	let arrN = arr ? Number(arr): 1;
	let info = informationsForTypes[type];
	if (!info)
		return {
			name,
			type,
			arr: arrN>1?arrN:0,
		};
	let [items,glType,ccType,mslType] = info;
	let size = items*arrN;
	// if exist glTMap, means the attribute is normalized, and the real gl type is GL_UNSIGNED_BYTE or GL_BYTE or ...
	// for example: in vec4 lightColorIn; //!< {GL_UNSIGNED_BYTE} 4 bytes, RGBA
	// let glTMap = mat[10]; //!< {GL_UNSIGNED_BYTE} or //!< {GL_BYTE} or ...
	let normalized = 'GL_FALSE';

	if (glTNormalized && glTNormalized != glType) {
		glType = glTNormalized;
		const normalizedType = normalizedForTypes[glType]
		if (normalizedType) {
			ccType = normalizedType.cc;
			mslType = normalizedType.msl[items-1] || 'MTLVertexFormatInvalid';
		}
		normalized = 'GL_TRUE';
	}
	return {
		name,
		items, // for example: vec4 has 4 items
		size: size, // for example: vec4 colors[256]; size = 4*256 = 1024
		type: type, // for example: vec4
		arr: arrN>1?arrN:0,
		glType: glType, // for example: GL_FLOAT
		ccType: ccType, // for example: float or int32_t
		mslType: mslType, // for example: MTLVertexFormatFloat4
		sizeOf: `sizeof(${ccType})*${size}`, // for example: sizeof(float)*1024
		normalized,
		...(location ? {location: Number(location)||0}: {})
	};
}

const all_import_sources = {};

// find uniform and attribute
// var reg = /^\s*(?:layout\s*\(\s*location\s*=\s*(\d+)\s*\)\s+)?
// (uniform|attribute|in)\s+((lowp|mediump|highp)\s+)?
// (int|float|vec2|vec3|vec4|mat2|mat3|mat4|sampler2D)
// \s+([a-zA-Z0-9\_]+)\s*(\[\s*(\d+)\s*\])?;\s*$/mg;
const find_regexp_attr = new RegExp(
	'^\\s*(?:layout\\s*\\(\\s*(location|binding)\\s*=\\s*(\\d+)\\s*\\)\\s+)?'+
	'(attribute|in)\\s+((lowp|mediump|highp)\\s+)?'+
	'(float|vec2|vec3|vec4|int|ivec2|ivec3|ivec4|uint|uvec2|uvec3|uvec4|mat2|mat3|mat4)'+
	'\\s+([a-zA-Z0-9\\_]+)\\s*(\\[\\s*(\\d+)\\s*\\])?;\\s*(?:\\/\\/\\!\\<\\s*\\{(.+?)\\}.*)?$'
	,'mg'
);
function find_attributes(code, attributes) {
	find_regexp_attr.lastIndex = 0; // reset index for global regexp
	var mat = find_regexp_attr.exec(code);

	while (mat) {
		let location = mat[2];
		let name = mat[7];
		let type = mat[6];
		let arr = mat[9];
		if (mat[3] == 'in') { // in
			// if exist glTMap, means the attribute is normalized, and the real gl type is GL_UNSIGNED_BYTE or GL_BYTE or ...
			// for example: in vec4 lightColorIn; //!< {GL_UNSIGNED_BYTE} 4 bytes, RGBA
			let normalized = mat[10]; //!< {GL_UNSIGNED_BYTE} or //!< {GL_BYTE} or ...
			attributes.push(
				parse_type_info(type, name, arr, location, normalized)
			);
		}

		mat = find_regexp_attr.exec(code);
	}
}

const find_regexp_uniform = new RegExp(
	'^\\s*(layout\\s*\\(\\s*binding\\s*=\\s*(\\d+)\\s*(,\\s*std(\\d+)\\s*)?\\)\\s+)?'+
	'(uniform|struct)\\s+((lowp|mediump|highp)\\s+)?'+
	'([a-zA-z0-9\\_\\$]+)'+ // type or struct name or sampler2D
	'('+
	'\\s+([a-zA-Z0-9\\_\\$]+)\\s*;|\\s*\\{'+ // name; or { for struct or uniform block
	'([^\\}]+)\\}' +
	')'
	,'mg'
);

function find_uniforms_blocks(code, uniforms, uniform_blocks, structs) {
	find_regexp_uniform.lastIndex = 0; // reset index for global regexp
	let mat = find_regexp_uniform.exec(code);

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
		let binding = mat[2] ? (Number(mat[2])||0) : 0;
		let std = mat[4];
		let key = mat[5]; // uniform or struct
		// let precision = mat[6]; // lowp or mediump or highp
		let type = mat[8]; // name for uniform, struct type name for struct
		let name = mat[10]; // name for uniform, empty for struct
		let block = mat[11]; // block content for uniform block, empty for uniform or struct
		if (key == 'uniform') { // uniform block or uniform
			if (block) { // uniform block
				uniform_blocks.push({
					type,
					name: type.substring(0, 1).toLowerCase() + type.substring(1), // for example: RootMatrixBlock -> rootMatrixBlock
					binding, std, block: parse_block(block),
				});
			} else if (type == 'sampler2D') {
				uniforms.push({
					type,
					name,
					glType: 'GL_SAMPLER_2D',
					nameSlot: name + 'Slot', // for example: textureSlot
					binding,
				});
			} else {
				// for example: uniform vec4 color; or uniform mat4 transform;
				// or uniform PcArgs pc; (PcArgs is struct type)
				uniforms.push({
					type,
					name,
					binding,
					std,
					...parse_type_info(type, name, '', '', ''),
				});
			}
		} else if (key == 'struct') { // struct
			if (block) {
				structs.push({
					type,
					name: type,
					block: parse_block(block)
				});
			}
		}

		mat = find_regexp_uniform.exec(code);
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
	let attributes = [];
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
	const msl_out = `${__dirname}/../src/render/shader/out/msl/${name}.${stage}.msl`;

	fs.writeFileSync(glsl_out, source, 'utf8');

	await exec2(`${glslc} -fshader-stage=${stage} ${glsl_out} -o ${spv_out}`);
	await exec2(`${spirv_cross} ${spv_out} --es --version 300 > ${es300_out}`);
	await exec2(`${spirv_cross} ${spv_out} --es --version 450 > ${es450_out}`);
	const metal_entry = `${name}_${stage}`;
	await exec2(`${spirv_cross} ${spv_out} --msl --msl-decoration-binding `+
		`--rename-entry-point main ${metal_entry} ${stage} --stage ${stage} > ${msl_out}`);

	const source_es300 = fs.readFileSync(es300_out).toString('utf8');
	const source_es450 = fs.readFileSync(es450_out).toString('utf8');
	const source_msl = fs.readFileSync(msl_out).toString('utf8');

	find_attributes(source, attributes);
	find_uniforms_blocks(source_es450, uniforms, uniform_blocks, structs);

	for (let struct of structs) {
		for (let b of struct.block) {
			if (!b.glType) {
				const type_struct = structs.find(s=>s.type == b.type);
				if (type_struct) {
					b.glType = 'struct ' + b.type;
					b.ccType = type_struct.type;
					b.struct = type_struct;
				}
			}
		}
	}

	for (let block of uniform_blocks) {
		for (let b of block.block) {
			if (!b.glType) {
				const type_struct = structs.find(s=>s.type == b.type);
				if (type_struct) {
					b.glType = 'struct ' + b.type;
					b.ccType = type_struct.type; // for example: PcArgs
					b.struct = type_struct;
				}
			}
		}
	}

	for (let uniform of uniforms) {
		if (!uniform.glType) {
			const type_struct = structs.find(s=>s.type == uniform.type);
			if (type_struct) {
				uniform.glType = 'struct ' + uniform.type;
				uniform.ccType = type_struct.type;
				uniform.struct = type_struct;
			}
		}
	}

	let ast = {
		name,
		stage,
		source,
		source_es300,
		source_msl,
		metal_entry,
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

	let structs = vert_ast.structs.concat(frag_ast.structs)
		.filter(e=>(set[e.type+'_struct'] ? 0: (set[e.type+'_struct']=1,1)));

	let uniforms = vert_ast.uniforms.concat(frag_ast.uniforms)
		.filter(e=>(set[e.name] ? 0: (set[e.name]=1,1)));

	// sort uniform blocks by binding index
	uniform_blocks.sort((a,b)=>a.binding-b.binding);
	uniforms.sort((a,b)=>a.binding-b.binding);

	let uniforms_commom = uniforms.filter(e=>!e.struct); // for: uniform vec4 color; or uniform Sampler2D texture;
	let uniforms_struct = uniforms.filter(e=>e.struct); // for: uniform PcArgs pc; (PcArgs is struct type)
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
			.replace(/\\/mg, '\\\\') // escape \ to \\, because we will put the code into c++ string, and \ is escape char in c++
			.replace(/"/mg, '\\"')
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
			doc.structs.concat(doc.uniform_blocks).map(s=>[
				`		struct ${s.type} {`,
					s.block.map(b=>
						`			${b.ccType} ${b.name}${b.items>1?`[${b.items}]`:''}${b.arr?`[${b.arr}]`:''}; // ${b.type}${b.arr?`[${b.arr}]`:''} ${b.name}`),
				`		};`
			]),
			doc.attributes.length ? `		GLuint ${doc.attributes.map(e=>e.name).join(',')}; // attributes location`: '',
			uniforms_commom.length ? `		GLuint ${uniforms_commom.map(e=>e.name).join(',')}; // uniforms location`: '',
			uniforms_struct.length ? uniforms_struct.map(e=>`		GLuint ${e.struct.block.map(it=>`${e.name}_${it.name}`).join(',')}; // struct uniform block location`) : '',
			uniforms_sampler2D.length ? `		GLuint ${uniforms_sampler2D.map(e=>e.nameSlot).join(',')}; // sampler2D texture slot`: '',
			doc.uniform_blocks.length ? `		GLuint ${doc.uniform_blocks.map(e=>e.name).join(',')}; // uniform block binding index`: '',
			`		virtual void build(const char* name, const char *macros);`,
		`	};`);

		// write cpp
		write(cpp, `void GLSL${doc.className}::build(const char* name, const char * macros) {`,
			`	gl_compile_link_shader(this, name,macros,`,
				`	${doc.vert_ast.glal_native_get_call},${doc.frag_ast.glal_native_get_call},`,
			'	{',
					doc.attributes.map(e=>`		{"${e.name}",${e.size},${e.glType},${e.sizeOf},${e.normalized},&${e.name}}, // ${e.type}${e.arr?`[${e.arr}]`:''} ${e.name}`),
			'	},',
			'	{',
					uniforms_commom.map(e=>`		{"${e.name}",${e.glType},&${e.name},${e.nameSlot?'&'+e.nameSlot:0}},`),
					uniforms_struct.map(e=>e.struct.block.map(it=>`		{"${e.name}.${it.name}",${it.glType},&${e.name}_${it.name},0},`)),
			'	},',
			'	{',
					doc.uniform_blocks.map(e=>`		{"${e.type}",&${e.name}},`),
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

// generate metal native code to cpp and hpp
function gen_mtl_native_code(glslDocs, output_h, output_mm) {
	var hpp = fs.openSync(output_h, 'w');
	var cpp = fs.openSync(output_mm, 'w');
	var now = Date.now();
	write(hpp,
		'// @private head',
		'#ifndef __mtl_shader_natives_' + now,
		'#define __mtl_shader_natives_' + now,
		'#include "./mtl_shader.h"',
		'namespace qk {',
	);
	// write pipeline kind enum
	write(hpp,
		'	enum MSLPipelineKind: uint8_t {',
				glslDocs.map(doc=>`		k${doc.className}_Pipeline,`),
				`		kPipelineCount`,
		'	};',
	);

	write(cpp,
		`#include "./${path.basename(output_h)}"`,
		'namespace qk {',
	);

	function write_cpp(ast, cpp) {
		let get_call = `get_${ast.name}()`;
		let code = ast.source_msl
			.replace(/^\s+/mg, '') // delete indent
			.replace(/\\/mg, '\\\\') // escape \ to \\, because we will put the code into c++ string, and \ is escape char in c++
			.replace(/"/mg, '\\"') // escape " to \", because we will put the code into c++ string, and " is escape char in c++
		let codeLen = Buffer.byteLength(code, 'utf8');

		ast.msl_native_get_call = `get_msl_${ast.name}_${ast.stage}`;

		write(cpp, `	String ${ast.msl_native_get_call}() {`,
			`		const char* c = "${code.replace(/\n/gm, '\\n\\\n')}";`,
			`		return String(c, ${codeLen});`,
		'	}',
		);
	}

	for (let doc of glslDocs) {
		write_cpp(doc.vert_ast, cpp);
		write_cpp(doc.frag_ast, cpp);

		const vertex = doc.vert_ast.uniforms.concat(doc.vert_ast.uniform_blocks);
		const fragment = doc.frag_ast.uniforms.concat(doc.frag_ast.uniform_blocks);

		let vertexBufferIndex = 0; // vertex buffer index
		for (let v of vertex) {
			vertexBufferIndex = Math.max(vertexBufferIndex, v.binding+1);
		}

		// write hpp
		write(hpp, `	struct MSL${doc.className}: MSLShader {`,
			doc.structs.concat(doc.uniform_blocks).map(s=>[
				`		struct ${s.type} {`,
					s.block.map(b=>
						typesForCC_Vec[b.type] ?
						`			${typesForCC_Vec[b.type]} ${b.name}${b.arr?`[${b.arr}]`:''}; // ${b.type}${b.arr?`[${b.arr}]`:''} ${b.name}` :
						`			${b.ccType} ${b.name}${b.items>1?`[${b.items}]`:''}${b.arr?`[${b.arr}]`:''}; // ${b.type}${b.arr?`[${b.arr}]`:''} ${b.name}`),
				`		};`
			]),
			vertex.length ?
			`		struct { uint32_t ${vertex.map(e=>`${e.name}${e.type=='sampler2D'?'/*sampler*/':''}`).join(',')}; } vertex; // vertex slot index` : '',
			fragment.length ?
			`		struct { uint32_t ${fragment.map(e=>`${e.name}${e.type=='sampler2D'?'/*sampler*/':''}`).join(',')}; } fragment; // fragment slot index` : '',
			`		void build();`,
			`	};`
		);

		// write build() function cpp
		write(cpp, `void MSL${doc.className}::build() {`,
			`	source = {"${doc.name}",k${doc.className}_Pipeline,${doc.vert_ast.msl_native_get_call},${doc.frag_ast.msl_native_get_call}};`,
			`	attributes = {`,
					doc.attributes.map(e=>{
						return `		{${vertexBufferIndex},${e.size},${e.mslType},${e.sizeOf}},`
					}),
			`	};`,
			`	bufferIndex = ${vertexBufferIndex};`,
				vertex.length ? `	vertex = { ${vertex.map(e=>`${e.binding}`).join(',')} };` : '',
				fragment.length ? `	fragment = { ${fragment.map(e=>`${e.binding}`).join(',')} };` : '',
			'}',
		);
	} // for (let doc of glslDocs) {

	write(hpp,
		`	struct MSLShaders {`,
	);
	write(cpp, `void MSLShaders::buildAll() {`);

	for (let doc of glslDocs) {
		write(hpp, `		MSL${doc.className} ${doc.name};`);
		write(cpp, `	${doc.name}.build();`);
		write(cpp, `	allShaders[k${doc.className}_Pipeline] = &${doc.name};`);
	}

	write(hpp,
		`		MSLShader* allShaders[kPipelineCount]; // all shaders, MSLPipelineKind => Shader pointer`,
		`		void buildAll();`,
		'	};',
		'}',
	'#endif');
	write(cpp, '}}');

	fs.closeSync(hpp);
	fs.closeSync(cpp);
}

function gen_vk_native_code(glslDocs, output_h, output_cc) {
	// TODO ...
}

async function main(output_gl_h,output_gl_cc) {
	if ( !check_file_is_change(inputs.concat([__filename]), [
		output_gl_h, output_gl_cc, output_mtl_h, output_mtl_mm, output_vk_h, output_vk_cc
	]) )
		return;

	console.log(process.cwd(), output_gl_h, output_gl_cc, output_mtl_h, output_mtl_mm, output_vk_h, output_vk_cc);

	for (let dir of ['glsl', 'es300', 'es450', 'spv', 'msl']) {
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

	if (output_gl_h && output_gl_cc) {
		gen_glsl_native_code(glslDocs, output_gl_h, output_gl_cc);
	}
	if (output_mtl_h && output_mtl_mm) {
		gen_mtl_native_code(glslDocs, output_mtl_h, output_mtl_mm);
	}
	if (output_vk_h && output_vk_cc) {
		// gen_vk_native_code(glslDocs, output_vk_h, output_vk_cc);
	}
}

main(output_gl_h||`${__dirname}/../src/render/gl/glsl_shaders.h`,
	output_gl_cc||`${__dirname}/../src/render/gl/glsl_shaders.cc`);
