#!/bin/sh

const fs = require('fs');

var pkg = JSON.parse(
	fs.readFileSync(__dirname + '/package.json', 'utf8')
);

pkg.types = pkg.main.replace(/\.js/, '.d.ts');

fs.writeFileSync(
	__dirname + `/out/${pkg.name}/package.json`,
	JSON.stringify(pkg, null, 2)
);

// gen native

const path = require('path');
const out = path.resolve(__dirname, 'out', pkg.name);
const nout = path.resolve(__dirname, '../../out');

if (!fs.existsSync(nout)) {
	fs.mkdirSync(nout);
}

for ( var i of ['_event', 'value', 'pkg', '_pkgutil'] ) {
	var j = i.substr(0, 1) == '_' ? i : '_' + i;
	fs.writeFileSync(`${nout}/${j}.js`, fs.readFileSync(`${out}/${i}.js`));
	fs.writeFileSync(`${out}/${i}.js`, `module.exports=__requireNgui__('${j}')`);
}