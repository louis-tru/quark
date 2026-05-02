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

const fs = require('fs');
const { execSync } = require('child_process');
const path = require('path');

const args = process.argv.slice(2);

const out_cc = args.pop();
const out_h = args.pop();
const inputs = args;

const shaderc = path.resolve('./bgfx/darwin/arm64/shaderc'); // 你的路径
const include = path.resolve('../../deps/bgfx/src');

function compileShader(input, type) {
    const name = path.basename(input).replace(/[\-\.]/g, '_');
    const out = name + '.bin';

    execSync(`${shaderc} \
        -f ${input} \
        -o ${out} \
        --platform osx \
        --type ${type} \
        --profile metal \
        -i ${include} \
        --varyingdef varying.def.sc`);

    return { name, file: out };
}

function bin2c(file, varname) {
    const data = fs.readFileSync(file);
    let arr = [];

    for (let i = 0; i < data.length; i++) {
        arr.push(`0x${data[i].toString(16).padStart(2,'0')}`);
    }

    return {
        array: arr,
        len: data.length,
        name: varname
    };
}

function writeShaders(hpp, cpp, shaders) {
    for (let s of shaders) {
        fs.writeSync(hpp, `extern const uint8_t ${s.name}[];\n`);
        fs.writeSync(hpp, `extern const uint32_t ${s.name}_len;\n\n`);

        fs.writeSync(cpp, `const uint8_t ${s.name}[] = {\n`);

        for (let i = 0; i < s.array.length; i++) {
            fs.writeSync(cpp, s.array[i] + ',');
            if ((i + 1) % 12 === 0) fs.writeSync(cpp, '\n');
            else fs.writeSync(cpp, ' ');
        }

        fs.writeSync(cpp, `\n};\n`);
        fs.writeSync(cpp, `const uint32_t ${s.name}_len = ${s.len};\n\n`);
    }
}

function main() {
    let shaders = [];

    for (let i = 0; i < inputs.length; i += 2) {
        const vs = inputs[i];
        const fs_ = inputs[i + 1];

        const vs_bin = compileShader(vs, 'vertex');
        const fs_bin = compileShader(fs_, 'fragment');

        const vs_c = bin2c(vs_bin.file, vs_bin.name);
        const fs_c = bin2c(fs_bin.file, fs_bin.name);

        shaders.push(vs_c, fs_c);
    }

    const hpp = fs.openSync(out_h, 'w');
    const cpp = fs.openSync(out_cc, 'w');

    fs.writeSync(hpp, '#pragma once\n#include <stdint.h>\n\n');
    fs.writeSync(cpp, `#include "${path.basename(out_h)}"\n\n`);

    writeShaders(hpp, cpp, shaders);

    fs.closeSync(hpp);
    fs.closeSync(cpp);

    console.log('Shader build complete');
}

main();