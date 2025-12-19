#!/usr/bin/env node

const fs = require('fs');
const path = require('path');
const pkg = JSON.parse(fs.readFileSync(__dirname + '/package.json', 'utf8'));

// gen native
const source = __dirname;
const out0 = path.resolve(source, 'out');
const out = path.resolve(source, 'out', pkg.name);
const out_types = path.resolve(source, 'out/@types', pkg.name);

pkg.types = pkg.main.replace(/\.js/, '.d.ts');
fs.writeFileSync(source + `/out/${pkg.name}/package.json`, JSON.stringify(pkg, null, 2));

function mkdirp(dir) {
	if (!fs.existsSync(dir)) {
		mkdirp(path.dirname(dir));
		fs.mkdirSync(dir);
	}
}

mkdirp(out);

for ( var i of ['_event', 'types', 'pkg', '_util', '_ext', 'uri'] ) {
	var j = i.substring(0, 1) == '_' ? i : '_' + i;
	fs.writeFileSync(`${out0}/${j}.js`, fs.readFileSync(`${out}/${i}.js`));
	fs.writeFileSync(`${out}/${i}.js`, `module.exports=__binding__('${j}')`);
}

function copyFiles(source, target, opts) {
	let stat = fs.statSync(source);
	if (stat.isFile()) {
		let buf = fs.readFileSync(source);
		mkdirp(path.dirname(target));
		fs.writeFileSync(target, opts.filter ? opts.filter(buf): buf);
	}
	else if ( stat.isDirectory() ) {
		let {include,includeExt,exclude,excludeExt} = opts;
		for (let i of fs.readdirSync(source)) {
			let ext = path.extname(i);
			if (include) {
				if (!include.has(i)) {
					if (includeExt) {
						if (!includeExt.has(ext)) 
							continue;
					} else {
						continue;
					}
				}
			} else if (includeExt) {
				if (!includeExt.has(ext))
					continue;
			}
			if (exclude && exclude.has(i))
				continue;
			if (excludeExt && excludeExt.has(ext))
				continue;
			copyFiles(source + '/' + i, target + '/' + i, opts);
		}
	}
}

function getFiles(source, ext) {
	let files = [];
	for (let i of fs.readdirSync(source)) {
		if ( ext.indexOf(path.extname(i)) != -1 ) {
			files.push(i);
		}
	}
	return files;
}

copyFiles(source, out, {includeExt:new Set(['.md','.json']),include:new Set(['LICENSE']),exclude:new Set(['out','tsconfig.json'])});
// copy publish @types/quark
copyFiles(out, out_types, {includeExt:new Set(['.ts','.md','.json']), include:new Set(['LICENSE'])});
copyFiles(out0, out0, {includeExt:new Set(['.js']), filter:e=>(e+'').replace(/require\("(\.|quark)\//gm, '__binding__("quark/')});
copyFiles(out, out, {includeExt:new Set(['.js']), filter:e=>(e+'').replace(/require\("(\.|quark)\//gm, '__binding__("quark/')});
// copyFiles(out, out, {includeExt:new Set(['.js']), filter:e=>('function require(name) { return __binding__(name.replace("./", "quark/")) }'+e)});

// gen gypi
fs.writeFileSync(`${source}/out/files.gypi`, JSON.stringify({
	'variables': {
		'libs_quark_ts_in': getFiles(source, ['.ts','.tsx','.json']).map(e=>`libs/quark/${e}`),
		'libs_quark_js_out': getFiles(out, ['.js','.json']).map(e=>`libs/quark/out/quark/${e}`),
	},
}, null, 2));

pkg.name = '@types/' + pkg.name;
fs.writeFileSync(`${out_types}/package.json`, JSON.stringify(pkg, null, 2));