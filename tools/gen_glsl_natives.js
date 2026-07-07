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
	int16_t: [1,'GL_SHORT','int16_t','MTLVertexFormatShort'],

	uint:  [1,'GL_UNSIGNED_INT','uint32_t','MTLVertexFormatUInt'],
	uvec2: [2,'GL_UNSIGNED_INT','uint32_t','MTLVertexFormatUInt2'],
	uvec3: [3,'GL_UNSIGNED_INT','uint32_t','MTLVertexFormatUInt3'],
	uvec4: [4,'GL_UNSIGNED_INT','uint32_t','MTLVertexFormatUInt4'],
	uint16_t: [1,'GL_UNSIGNED_SHORT','uint16_t','MTLVertexFormatUShort'],

	mat2:  [4,'GL_FLOAT','float','MTLVertexFormatFloat4'], // 2*2 = 4
	mat3:  [9,'GL_FLOAT','float','MTLVertexFormatInvalid'],
	mat4:  [16,'GL_FLOAT','float','MTLVertexFormatInvalid'],
	sampler2D: [1,'GL_INT','int','MTLVertexFormatInt'],

	bool: [1,'GL_BOOL','uint32_t','MTLVertexFormatUInt'],
};

// for example: float for float, vec4 for Vec4, int for int32_t, ivec3 for IVec3, ...
// because in metal, vec3 is not 12 bytes, but 16 bytes, 
// so we need to use Vec3Padding for vec3, and IVec3Padding for ivec3
const typesForMSL_Vec = {
	vec2: 'Vec2',
	vec3: 'Vec3Padding',
	vec4: 'Vec4',
	ivec2: 'IVec2',
	ivec3: 'IVec3Padding',
	ivec4: 'IVec4',
	mat4: 'Mat4',
};
const typesForGLSL_Vec = {
	vec2: 'Vec2',
	vec3: 'Vec3',
	vec4: 'Vec4',
	ivec2: 'IVec2',
	ivec3: 'IVec3',
	ivec4: 'IVec4',
	mat4: 'Mat4',
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
				parse_type_info(type, name, arr, location||'0', normalized)
			);
		}

		mat = find_regexp_attr.exec(code);
	}
}

const find_regexp_uniform = new RegExp(
	'^\\s*(?:layout\\s*\\(([^\\)]*)\\)\\s+)?'+
	'(?:(readonly|writeonly|coherent|restrict|volatile)\\s+)*'+
	'(uniform|buffer|struct)\\s+'+
	'(?:(readonly|writeonly|coherent|restrict|volatile)\\s+)*'+
	'(?:(lowp|mediump|highp)\\s+)?'+
	'([a-zA-Z0-9\\_\\$]+)'+ // type or block name
	'(?:\\s+([a-zA-Z0-9\\_\\$]+)\\s*(\\[\\s*([^\\]]*)\\s*\\])?\\s*;|\\s*\\{([^\\}]+)\\}\\s*([a-zA-Z0-9\\_\\$]+)?\\s*;?)'
	,'mg'
);

const glTypesForTextures = {
	sampler2D: 'GL_SAMPLER_2D', // combined image sampler
	texture2D: 'GL_TEXTURE_2D', // sampled image
	sampler: 'GL_SAMPLER', // sampler state
	image2D: 'GL_IMAGE_2D', // storage image
};

// Existing shaders use set=0 for buffers and set=1 for ordinary texture slots.
// Runtime opaque arrays in higher sets are emitted through Metal argument buffers.
const mslDiscreteDescriptorSets = new Set([0, 1]);
const mslArgumentBufferSets = [2, 3, 4, 5, 6, 7, 8];
const mslArgumentBufferOptions =
	'--msl-argument-buffers --msl-argument-buffer-tier 1 ' +
	'--msl-discrete-descriptor-set 0 --msl-discrete-descriptor-set 1 ' +
	mslArgumentBufferSets.map(set=>`--msl-device-argument-buffer ${set}`).join(' ') + ' ';

function parse_layout_set(layout) {
	const setMatch = layout.match(/\bset\s*=\s*(\d+)/);
	return setMatch ? Number(setMatch[1]) || 0: 0;
}

function parse_layout_binding(layout) {
	const bindingMatch = layout.match(/\bbinding\s*=\s*(\d+)/);
	return bindingMatch ? Number(bindingMatch[1]) || 0: 0;
}

function parse_array_info(arrFull, arrValue) {
	return {
		arrayCount: arrValue ? Number(arrValue) || 0: 0,
		runtimeArray: !!arrFull && !arrValue,
	};
}

function is_opaque_resource(type) {
	return !!glTypesForTextures[type];
}

function is_msl_argument_resource(e) {
	return is_opaque_resource(e.type) && e.runtimeArray && !mslDiscreteDescriptorSets.has(e.set || 0);
}

function collect_msl_argument_sets(types) {
	const argumentTypes = types.filter(e=>is_msl_argument_resource(e));
	const argumentSets = Array.from(argumentTypes.reduce((sets, e)=>(sets.add(e.set), sets), new Set()))
		.sort((a,b)=>a-b)
		.map(set=>({
			set,
			resources: argumentTypes.filter(e=>e.set == set).sort((a,b)=>a.binding-b.binding),
		}))
	return argumentSets;
}

function parse_block(blockStr) {
	// struct PcArgs
	// {
	// 		mediump float depth;
	// 		mediump float allScale;
	// 		mediump vec4 texCoords;
	// 		mediump vec4 color;
	// };
	const block = [];
	const reg = /^\s*((mediump|lowp|highp)\s+)?([a-zA-Z0-9\_\$]+)\s+([a-zA-Z0-9\_]+)\s*(\[\s*(\d*)\s*\])?;\s*$/mg;
	let mat = reg.exec(blockStr);

	while (mat) {
		const type = mat[3];
		const name = mat[4];
		const arr = mat[6]; // vec4 colors[256]; arr = 256
		const runtimeArray = mat[5] && !arr;
		const info = {
			...parse_type_info(type, name, runtimeArray ? 1: arr, '', ''),
			runtimeArray,
		};
		if (runtimeArray)
			info.arr = 1;
		block.push(info);
		mat = reg.exec(blockStr);
	}
	return block;
}

function find_uniforms(code, uniforms, uniform_blocks, storage_blocks, structs) {
	find_regexp_uniform.lastIndex = 0; // reset index for global regexp
	let mat = find_regexp_uniform.exec(code);

	while (mat) {
		const layout = mat[1] || '';
		const stdMatch = layout.match(/\bstd(\d+)/);
		let binding = parse_layout_binding(layout);
		let set = parse_layout_set(layout);
		let std = stdMatch ? stdMatch[1]: '';
		let key = mat[3]; // uniform, buffer or struct
		let access = mat[4] || mat[2] || '';
		let type = mat[6]; // type or block name
		let name = mat[7] || mat[11]; // simple resource name or block instance name
		let typeLower = type.substring(0, 1).toLowerCase() + type.substring(1); // for example: PcArgs to pcArgs
		let blockName = !name || /^_\d+$/.test(name) ? typeLower: name;
		let arrFull = mat[8];
		let arrValue = mat[9];
		let arrayInfo = parse_array_info(arrFull, arrValue);
		let block = mat[10]; // block content
		if (key == 'uniform') { // uniform block or uniform
			if (block) { // uniform block
				uniform_blocks.push({
					type,
					rawName: name,
					name: blockName,
					binding,
					set,
					std,
					access,
					block: parse_block(block),
				});
			} else if (glTypesForTextures[type]) { // uniform sampler2D or uniform texture2D
				if (arrayInfo.arrayCount == 1) {
					arrayInfo.arrayCount = 0;
					arrayInfo.runtimeArray = true;
				}
				uniforms.push({
					type,
					name,
					glType: glTypesForTextures[type],
					nameSlot: name + 'Slot', // for example: textureSlot
					binding,
					set,
					access,
					...arrayInfo,
				});
			} else {
				// for example: uniform vec4 color; or uniform mat4 transform;
				// or uniform PcArgs pc; (PcArgs is struct type)
				uniforms.push({
					type,
					name,
					binding,
					set,
					std,
					access,
					...parse_type_info(type, name, '', '', ''),
				});
			}
		} else if (key == 'buffer' && block) {
			storage_blocks.push({
				type,
				rawName: name,
				name: blockName,
				binding,
				set,
				std,
				access,
				storage: true,
				block: parse_block(block),
			});
		} else if (key == 'struct') { // struct
			if (block) {
				structs.push({
					type,
					name: typeLower,
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

	// A document may optionally append one compute stage. Keep this order fixed:
	// shared utility, #vert, #frag, #comp.
	let [renderstr, compstr] = codestr.split(/^#comp/gm);
	let [first, fragstr] = renderstr.split(/^#frag/gm); // split vert and frag
	let [util, vertstr] = first.split(/^#vert/gm); // split util and vert
	let hasRender = vertstr != undefined || fragstr != undefined;
	let hasComp = compstr != undefined;

	vertstr = beforeCode + util + (vertstr || ''); // util + vert
	fragstr = beforeCode + util + (fragstr || ''); // util + frag
	compstr = beforeCode + util + (compstr || ''); // util + comp

	all_import_sources[pathname] = result; // cache source, for example: {filename, vert, frag, comp}

	result['vert'] = resolve_source_part(dirname, vertstr);
	result['frag'] = resolve_source_part(dirname, fragstr);
	// Even files without #comp may provide shared code before #vert that an
	// importing compute shader needs. hasComp only controls top-level output.
	result['comp'] = resolve_source_part(dirname, compstr);
	result['hasRender'] = hasRender;
	result['hasComp'] = hasComp;

	return result;
}

function marge_source(stage, source, output, import_set) {
	for (let part of source) {
		if (typeof part == 'string') {
			output.push(part);
		} else if (!import_set.has(part.filename)) { // if not import, marge source, else ignore
			import_set.add(part.filename);
			marge_source(stage, part[stage], output, import_set);
		}
	}
}

function define_stage_macro(source_arr, stage) {
	const define = `#define Qk_${stage.toUpperCase()} 1\n`;
	let source = source_arr[0] || '';
	// GLSL requires #version to remain the first directive.
	const version = source.match(/^#version[^\n]*(?:\n|$)/);
	source_arr[0] = version ?
		version[0] + define + source.substring(version[0].length):
		define + source;
}

function readMSLSource(msl_out, uniforms) {
	let source_msl = fs.readFileSync(msl_out).toString('utf8');
	if (uniforms.find(e=>e.runtimeArray)) {
		// fix PcArgs buffer to [[buffer(0)]]
		let isReplaced = false;
		const code = source_msl.replace(/PcArgs\&\s+pc\s+\[\[buffer\(\d+\)\]\]/, e=>{
			isReplaced = true;
			return 'PcArgs& pc [[buffer(0)]]';
		});
		if (isReplaced) {
			source_msl = code;
			fs.writeFileSync(msl_out, source_msl, 'utf8');
		}
	}
	return source_msl;
}

async function resolve_ast(name, stage, source_both) {
	const source_arr = [];
	marge_source(stage, source_both[stage], source_arr, new Set());

	define_stage_macro(source_arr, stage);
	let source = source_arr.join('');
	let attributes = [];
	let if_flags = [];
	let uniforms = [];
	let uniform_blocks = [];
	let storage_blocks = [];
	let structs = [];

	let if_reg = / Qk_SHADER_IF_FLAGS_([a-z0-9\_]+)/igm,if_m;
	// query if flags
	while (if_m = if_reg.exec(source)) {
		if_flags.push(if_m[1]);
	}

	const glsl_out = `${__dirname}/../src/render/shader/out/glsl/${name}.${stage}.glsl`;
	const spv_out = `${__dirname}/../src/render/shader/out/spv/${name}.${stage}.spv`;
	const spv_es300_out = `${__dirname}/../src/render/shader/out/spv/${name}.${stage}.es300.spv`;
	const gl450_out = `${__dirname}/../src/render/shader/out/gl450/${name}.${stage}.gl450.glsl`;
	const es300_out = `${__dirname}/../src/render/shader/out/es300/${name}.${stage}.es300.glsl`;
	const msl_out = `${__dirname}/../src/render/shader/out/msl/${name}.${stage}.metal`;

	fs.writeFileSync(glsl_out, source, 'utf8');

	await exec2(`${glslc} -DQk_SHADER_FLAGS_ENABLE_CGAA=1 -DQk_SHADER_FLAGS_ENABLE_CAPA=1 -fshader-stage=${stage} ${glsl_out} -o ${spv_out}`);
	if (stage != 'comp') {
		await exec2(`${glslc} -fshader-stage=${stage} ${glsl_out} -o ${spv_es300_out}`);
		await exec2(`${spirv_cross} ${spv_es300_out} --es --version 300 > ${es300_out}`);
	}
	await exec2(`${spirv_cross} ${spv_out} --vulkan-semantics --version 450 > ${gl450_out}`);
	const metal_entry = `${name}_${stage}`;
	let mslOptions = `--msl-version 23000 --msl-decoration-binding ${mslArgumentBufferOptions}`;
	await exec2(`${spirv_cross} ${spv_out} --msl ${mslOptions}`+
		`--rename-entry-point main ${metal_entry} ${stage} --stage ${stage} > ${msl_out}`);

	const source_es300 = stage == 'comp' ? '': fs.readFileSync(es300_out).toString('utf8');
	const source_gl450 = fs.readFileSync(gl450_out).toString('utf8');

	find_attributes(source, attributes);
	find_uniforms(source_gl450, uniforms, uniform_blocks, storage_blocks, structs);

	var linked = new Set();
	function link_type(owner) {
		if (linked.has(owner))
			return;
		linked.add(owner);
		for (let b of owner.block) {
			if (!b.glType) {
				const type_struct = structs.find(s=>s.type == b.type);
				if (type_struct) {
					b.glType = 'struct ' + b.type;
					b.ccType = type_struct.type;
					b.struct = type_struct;
					if (owner.storage)
						type_struct.storage = true;
				}
			}
			if (b.block) {
				link_type(b);
			}
		}
	}

	for (let owner of storage_blocks.concat(uniform_blocks, structs)) {
		link_type(owner);
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
		source_gl450,
		source_msl: readMSLSource(msl_out, uniforms),
		metal_entry,
		structs,
		if_flags,
		attributes: attributes.sort((a,b)=>a.location-b.location),
		uniforms: uniforms.sort((a,b)=>a.set-b.set || a.binding-b.binding),
		uniform_blocks: uniform_blocks.sort((a,b)=>a.set-b.set || a.binding-b.binding),
		storage_blocks: storage_blocks.sort((a,b)=>a.set-b.set || a.binding-b.binding),
	};
	return ast;
}

async function resolve_doc(name_, input) {
	const source_both = resolve_source_both(`#import "${__dirname}/../src/render/shader/_util.glsl"\n`, input);
	const name = name_.replace(/[\-_](.)/gm, (_,b)=>b.toUpperCase());
	const className = `${name[0].toUpperCase()}${name.substring(1)}`;
	const vert_ast = source_both.hasRender ? await resolve_ast(name, 'vert', source_both): null;
	const frag_ast = source_both.hasRender ? await resolve_ast(name, 'frag', source_both): null;
	const comp_ast = source_both.hasComp ? await resolve_ast(name, 'comp', source_both): null;
	const render_asts = vert_ast && frag_ast ? [vert_ast, frag_ast]: [];
	const asts = render_asts.concat(comp_ast ? [comp_ast]: []);

	let set = {};
	let structs = render_asts.flatMap(e=>e.structs)
		.filter(e=>(set[e.type+'_struct'] ? 0: (set[e.type+'_struct']=1,1)));

	let uniform_blocks = render_asts.flatMap(e=>e.uniform_blocks)
		.filter(e=>(set[e.type+'_block'] ? 0: (set[e.type+'_block']=1,1))).sort((a,b)=>a.set-b.set || a.binding-b.binding);

	let storage_blocks = render_asts.flatMap(e=>e.storage_blocks)
		.filter(e=>(set[e.type+'_storage'] ? 0: (set[e.type+'_storage']=1,1)));

	let uniforms = render_asts.flatMap(e=>e.uniforms)
		.filter(e=>(set[e.name] ? 0: (set[e.name]=1,1))).sort((a,b)=>a.set-b.set || a.binding-b.binding);

	set = {};
	let metal_structs = asts.flatMap(e=>e.structs)
		.filter(e=>(set[e.type+'_struct'] ? 0: (set[e.type+'_struct']=1,1)));

	let metal_uniform_blocks = asts.flatMap(e=>e.uniform_blocks)
		.filter(e=>(set[e.type+'_block'] ? 0: (set[e.type+'_block']=1,1)));

	let metal_storage_blocks = asts.flatMap(e=>e.storage_blocks)
		.filter(e=>(set[e.type+'_storage'] ? 0: (set[e.type+'_storage']=1,1)));

	let if_flags = asts.flatMap(e=>e.if_flags)
		.reduce((a,i)=>((a.indexOf(i)==-1?a.push(i):void 0),a), []);

	return { // return doc
		name, // for example: colorRadial
		className, // for example: ColorRadial
		source_both,
		vert_ast, //
		frag_ast, //
		comp_ast, //
		attributes: vert_ast ? vert_ast.attributes: [], // doc vert attributes
		structs, // only reader defined structs
		uniform_blocks, // only reader defined uniform blocks
		storage_blocks,
		uniforms, // only reader defined uniforms
		metal_structs, // all structs
		metal_uniform_blocks, // all uniform blocks
		metal_storage_blocks, // all storage blocks
		if_flags, // doc all if flags
		glal_native_get_call: '',
	};
}

// generate glsl native code to cpp and hpp
function gen_glsl_native_code(glslDocs, output_h, output_cc) {
	// filter docs, only keep docs that have vert and frag ast, and no storage blocks
	glslDocs = glslDocs.filter(doc=>doc.vert_ast && doc.frag_ast/*&& doc.storage_blocks.length == 0*/);
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

		const uniforms = doc.uniforms;
		const uniforms_commom = uniforms.filter(e=>!e.struct); // for: uniform vec4 color; or uniform sampler2D texture;
		const uniforms_struct = uniforms.filter(e=>e.struct) // for: uniform PcArgs pc; (PcArgs is struct type)
			.concat(doc.uniform_blocks.filter(e=>e.name=='pc'?(e.struct=e,true):false));
		const uniforms_sampler2D = uniforms.filter(e=>e.glType == 'GL_SAMPLER_2D');
		const uniform_blocks = doc.uniform_blocks.filter(e=>e.name!='pc');

		// write hpp
		write(hpp, `	struct GLSL${doc.className}: GLSLShader {`,
			doc.structs.concat(uniform_blocks).map(s=>[
				`		struct ${s.storage ? '': 'alignas(16) '}${s.type} {`,
					s.block.map(b=>
						(typesForGLSL_Vec[b.type] ?
						`			${typesForGLSL_Vec[b.type]} ${b.name}${b.arr?`[${b.arr}]`:''};` :
						`			${b.ccType} ${b.name}${b.items>1?`[${b.items}]`:''}${b.arr?`[${b.arr}]`:''};`) +
						` // ${b.type}${b.runtimeArray?'[]':b.arr?`[${b.arr}]`:''} ${b.name}${b.runtimeArray?' (runtime array, placeholder length 1)':''}`
					),
				`		};`
			]),
			doc.attributes.length ? `		GLint ${doc.attributes.map(e=>e.name).join(',')}; // attributes location`: '',
			uniforms_commom.length ? `		GLint ${uniforms_commom.map(e=>e.name).join(',')}; // uniforms location`: '',
			uniforms_struct.length ? uniforms_struct.map(e=>`		GLint ${e.struct.block.map(it=>`${e.name}_${it.name}`).join(',')};`) : '',
			uniforms_sampler2D.length ? `		GLint ${uniforms_sampler2D.map(e=>e.nameSlot).join(',')}; // sampler2D texture slot`: '',
			uniform_blocks.length ? `		GLint ${uniform_blocks.map(e=>e.name).join(',')}; // uniform block binding index`: '',
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
					uniform_blocks.map(e=>`		{"${e.type}",&${e.name}},`),
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

	function defineSlotIndexs(types, name, indent) {
		if (!types.length)
			return '';
		const simpleTypes = types.filter(e=>!is_msl_argument_resource(e));
		const argumentSets = collect_msl_argument_sets(types);
		return [
			`${indent}struct {`,
			simpleTypes.length ? `${indent}	uint32_t ${simpleTypes.map(e=>`${e.name}/*${e.type}*/`).join(',\n'+indent+'	')};`: '',
			argumentSets.map(s=>[
				`${indent}	struct {`,
				`${indent}		uint32_t bufferIndex;`,
				s.resources.map(e=>`${indent}		MSLArgumentResource ${e.name}; // ${e.type}${e.runtimeArray?'[]':e.arrayCount?`[${e.arrayCount}]`:''}`),
				`${indent}	} set${s.set};`,
			]),
			`${indent}} ${name}; // ${name} slot index`,
		];
	}

	function argumentResourceValue(e) {
		return `{${e.binding},${e.arrayCount||0},${e.runtimeArray?'true':'false'}}`;
	}

	function slotIndexInit(types) {
		const simpleTypes = types.filter(e=>!is_msl_argument_resource(e));
		const argumentSets = collect_msl_argument_sets(types);
		return simpleTypes.map(e=>`${e.binding}`).concat(
			argumentSets.map(s=>`{${s.set},${s.resources.map(argumentResourceValue).join(',')}}`)
		);
	}

	for (let doc of glslDocs) {
		if (doc.vert_ast)
			write_cpp(doc.vert_ast, cpp);
		if (doc.frag_ast)
			write_cpp(doc.frag_ast, cpp);
		if (doc.comp_ast)
			write_cpp(doc.comp_ast, cpp);

		const blocks = doc.metal_structs.concat(doc.metal_uniform_blocks, doc.metal_storage_blocks);
		const vertex = doc.vert_ast ? doc.vert_ast.uniforms.concat(doc.vert_ast.uniform_blocks, doc.vert_ast.storage_blocks): [];
		const fragment = doc.frag_ast ? doc.frag_ast.uniforms.concat(doc.frag_ast.uniform_blocks, doc.frag_ast.storage_blocks): [];
		const compute = doc.comp_ast ? doc.comp_ast.uniforms.concat(doc.comp_ast.uniform_blocks, doc.comp_ast.storage_blocks): [];

		let vertexBufferIndex = 0; // vertex buffer index
		for (let v of vertex) {
			vertexBufferIndex = Math.max(vertexBufferIndex, (is_msl_argument_resource(v) ? v.set: v.binding) + 1);
		}

		// write hpp
		write(hpp, `	struct MSL${doc.className}: MSLShader {`,
			blocks.map(s=>[
				`		struct ${s.storage ? '': 'alignas(16) '}${s.type} {`,
					s.block.map(b=>
						(typesForMSL_Vec[b.type] ?
						`			${typesForMSL_Vec[b.type]} ${b.name}${b.arr?`[${b.arr}]`:''};` :
						`			${b.ccType} ${b.name}${b.items>1?`[${b.items}]`:''}${b.arr?`[${b.arr}]`:''};`) +
						` // ${b.type}${b.runtimeArray?'[]':b.arr?`[${b.arr}]`:''} ${b.name}${b.runtimeArray?' (runtime array, placeholder length 1)':''}`
					),
				`		};`
			]),
			defineSlotIndexs(vertex, 'vertex', '		'),
			defineSlotIndexs(fragment, 'fragment', '		'),
			defineSlotIndexs(compute, 'compute', '		'),
			`		void build();`,
			`	};`
		);

		// write build() function cpp
		write(cpp, `void MSL${doc.className}::build() {`,
			`	source = {"${doc.name}",k${doc.className}_Pipeline,${doc.vert_ast ? doc.vert_ast.msl_native_get_call: 'nullptr'},`+
				`${doc.frag_ast ? doc.frag_ast.msl_native_get_call: 'nullptr'},${doc.comp_ast ? doc.comp_ast.msl_native_get_call: 'nullptr'}};`,
			`	attributes = {`,
					doc.attributes.map(e=>{
						return `		{${vertexBufferIndex},${e.size},${e.mslType},${e.sizeOf}},`
					}),
			`	};`,
			`	bufferIndex = ${vertexBufferIndex};`,
				vertex.length ? `	vertex = { ${slotIndexInit(vertex).join(',')} };` : '',
				fragment.length ? `	fragment = { ${slotIndexInit(fragment).join(',')} };` : '',
				compute.length ? `	compute = { ${slotIndexInit(compute).join(',')} };` : '',
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

	for (let dir of ['glsl', 'es300', 'gl450', 'spv', 'msl']) {
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
