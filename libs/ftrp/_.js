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
