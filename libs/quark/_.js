#!/bin/sh

const fs = require('fs');
const path = require('path');

var pkg = JSON.parse(
	fs.readFileSync(__dirname + '/package.json', 'utf8')
);

pkg.types = pkg.main.replace(/\.js/, '.d.ts');

// gen native
const out = path.resolve(__dirname, 'out', pkg.name);
const out_ = path.resolve(__dirname, '../../out');
const out_types = path.resolve(__dirname, 'out/@types', pkg.name);

fs.writeFileSync(
	__dirname + `/out/${pkg.name}/package.json`,
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

for ( var i of ['_event', 'value', 'pkg', '_pkgutil'] ) {
	var j = i.substring(0, 1) == '_' ? i : '_' + i;
	fs.writeFileSync(`${out_}/${j}.js`, fs.readFileSync(`${out}/${i}.js`));
	fs.writeFileSync(`${out}/${i}.js`, `module.exports=__require__('${j}')`);
}

// publish @types/quark

const copy_types_ext = {
	'.ts': 1,
	'.md': 1,
	'.json': 1,
}

function copy_types(source, target) {
	var stat = fs.statSync(source);
	if (stat.isFile()) {
		mkdirp(path.dirname(target));
		fs.writeFileSync(target, fs.readFileSync(source));
	} else if ( stat.isDirectory() ) {
		for (var i of fs.readdirSync(source)) {
			if ( i == 'LICENSE' || copy_types_ext.hasOwnProperty(path.extname(i)) ) {
				copy_types(source + '/' + i, target + '/' + i);
			}
		}
	}
}

mkdirp(out_types);
copy_types(out, out_types);

pkg.name = '@types/' + pkg.name;

fs.writeFileSync(`${out_types}/package.json`, JSON.stringify(pkg, null, 2));