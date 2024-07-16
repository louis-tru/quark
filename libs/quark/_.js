#!/bin/sh

const fs = require('fs');
const path = require('path');

var pkg = JSON.parse(
	fs.readFileSync(__dirname + '/package.json', 'utf8')
);

pkg.types = pkg.main.replace(/\.js/, '.d.ts');

// gen native
const source = __dirname;
const out = path.resolve(source, 'out', pkg.name);
const out_ = path.resolve(source, 'out');
const out_types = path.resolve(source, 'out/@types', pkg.name);

fs.writeFileSync(
	source + `/out/${pkg.name}/package.json`,
	JSON.stringify(pkg, null, 2)
);

function mkdirp(dir) {
	if (!fs.existsSync(dir)) {
		mkdirp(path.dirname(dir));
		fs.mkdirSync(dir);
	}
}

mkdirp(out);
mkdirp(out_);

for ( var i of ['_event', 'types', 'pkg', '_util', '_ext'] ) {
	var j = i.substring(0, 1) == '_' ? i : '_' + i;
	fs.writeFileSync(`${out_}/${j}.js`, fs.readFileSync(`${out}/${i}.js`));
	fs.writeFileSync(`${out}/${i}.js`, `module.exports=__binding__('${j}')`);
}

function copy_files(source, target, ext) {
	var stat = fs.statSync(source);
	if (stat.isFile()) {
		mkdirp(path.dirname(target));
		fs.writeFileSync(target, fs.readFileSync(source));
	} else if ( stat.isDirectory() ) {
		for (var i of fs.readdirSync(source)) {
			if ( i == 'LICENSE' || ext.indexOf(path.extname(i)) != -1 ) {
				copy_files(source + '/' + i, target + '/' + i, ext);
			}
		}
	}
}

function get_files(source, ext) {
	var files = [];
	for (var i of fs.readdirSync(source)) {
		if ( ext.indexOf(path.extname(i)) != -1 ) {
			files.push(i);
		}
	}
	return files;
}

mkdirp(out_types);
// copy publish @types/quark
copy_files(out, out_types, ['.ts','.md','.json']);

// gen gypi
fs.writeFileSync(`${source}/out/files.gypi`, JSON.stringify({
	'variables': {
		'libs_quark_ts_in': get_files(source, ['.ts','.tsx','.json']).map(e=>`libs/quark/${e}`),
		'libs_quark_js_out': get_files(out, ['.js','.json']).map(e=>`libs/quark/out/quark/${e}`),
	},
}, null, 2));

pkg.name = '@types/' + pkg.name;

fs.writeFileSync(`${out_types}/package.json`, JSON.stringify(pkg, null, 2));