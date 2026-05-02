#!/usr/bin/env node

const fs = require('../../tools/node_modules/qktool/node/fs2');
const { exec } = require('../../tools/node_modules/qktool/node/syscall');
const path = require('path');

async function exec2(cmd) {
	var r = await exec(cmd, {
		stdout: process.stdout,
		stderr: process.stderr, stdin: process.stdin,
	});
	if (r.code != 0) {
		throw Error.new(`Run cmd fail, "${cmd}", code = ${r.code}`);
	}
}

const shaderc = path.resolve(`${__dirname}/../../tools/bgfx/${process.platform}/${process.arch}/shaderc`); // arch should be arm64 or x64, depends on your machine
const include = path.resolve(`${__dirname}/../../deps/bgfx/src`);

const shaders_in = [
	{ input: 'test-v.sc', type: 'vertex' },
	{ input: 'test-f.sc', type: 'fragment' }
];

async function compileShader(input, type) {
	const name = path.basename(input).replace(/[\-\.]/g, '_');
	const bin = `${__dirname}/${name}.bin`;

	// ../../tools/bgfx/darwin/arm64/shaderc -f test-v.sc -o test-v.bin --platform osx --type vertex --profile metal  -i ../../deps/bgfx/src
	// ../../tools/bgfx/darwin/arm64/shaderc -f test-f.sc -o test-f.bin --platform osx --type fragment --profile metal  -i ../../deps/bgfx/src

	await exec2(`${shaderc} \
		-f ${__dirname}/${input} \
		-o ${bin} \
		--platform osx \
		--type ${type} \
		--profile metal \
		-i ${include} \
		--varyingdef ${__dirname}/varying.def.sc`);

	// --profile 150 for OpenGL 3.2, --profile metal for Metal, --profile 300 es for OpenGL ES 3.0

	return { name, bin };
}

async function main(params) {
	const shaders = []; // [{ name, bin }]
	for (const s of shaders_in) {
		shaders.push(await compileShader(s.input, s.type));
	}

	console.log('Compile shaders done:', shaders);

	const hpp = `shader_data.h`;
	const cpp = `shader_data.cpp`;

	let hppContent = [
		`#ifndef SHADER_DATA_H`,
		`#define SHADER_DATA_H`,
		`#include <stdint.h>`,
		`#pragma once`
	];
	let cppContent = [
		`#include "${hpp}"`
	];

	for (const s of shaders) {
		const buf = await fs.readFile(s.bin);
		const arr = [];

		for (let ch of buf) {
			const hex = ch.toString(16).padStart(2, '0');
			arr.push(`0x${hex}`);
		}

		hppContent.push(`extern const uint8_t ${s.name}[];`);
		hppContent.push(`extern const uint32_t ${s.name}_len;\n`);

		cppContent.push(`const uint8_t ${s.name}[] = {\n${arr.join(', ')}\n};\n`);
		cppContent.push(`const uint32_t ${s.name}_len = ${buf.length};\n`);

		await fs.remove(s.bin);
	}

	hppContent.push(`#endif // SHADER_DATA_H`);

	await fs.writeFile(`${__dirname}/${hpp}`, hppContent.join('\n'));
	await fs.writeFile(`${__dirname}/${cpp}`, cppContent.join('\n'));
}

main().catch(e => {
	console.error(e);
	process.exit(1);
});