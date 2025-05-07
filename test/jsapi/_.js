
const fs = require('fs');
const path = require('path');
const pkg = JSON.parse(fs.readFileSync(__dirname + '/package.json', 'utf8'));
const source = __dirname;
const out = path.resolve(source, 'out', pkg.name);

function mkdirp(dir) {
	if (!fs.existsSync(dir)) {
		mkdirp(path.dirname(dir));
		fs.mkdirSync(dir);
	}
}

function copyFiles(source, target, opts) {
	let stat = fs.statSync(source);
	if (stat.isFile()) {
		let buf = fs.readFileSync(source);
		mkdirp(path.dirname(target));
		fs.writeFileSync(target, opts.filter ? opts.filter(buf): buf);
	} else if ( stat.isDirectory() ) {
		let {include,exclude,includeExt,excludeExt} = opts;
		for (let i of fs.readdirSync(source)) {
			let ext = path.extname(i);
			if (include) {
				if (!include.has(i)) {
					if (includeExt) {
						if (!includeExt.has(ext)) continue;
					} else {
						continue;
					}
				}
			} else if (includeExt) {
				if (!includeExt.has(ext)) continue;
			}
			if (exclude && exclude.has(i)) continue;
			if (excludeExt && excludeExt.has(ext)) continue;
			copyFiles(source + '/' + i, target + '/' + i, opts);
		}
	}
}

copyFiles(source, out, {
	excludeExt: new Set(['.ts','.tsx','.js']),
	exclude: new Set(['out','_.js','tsconfig.json','ex']),
});

function getFiles(source, ext) {
	let files = [];
	for (let i of fs.readdirSync(source)) {
		if ( ext.indexOf(path.extname(i)) != -1 ) {
			files.push(i);
		}
	}
	return files;
}

// gen gypi
fs.writeFileSync(`${source}/out/files.gypi`, JSON.stringify({
	'variables': {
		'jsapi_in': getFiles(source, ['.ts','.tsx','.json']).map(e=>`test/jsapi/${e}`),
		'jsapi_out': getFiles(out, ['.js','.json']).map(e=>`test/jsapi/out/jsapi/${e}`),
	},
}, null, 2));