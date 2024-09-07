
import * as fs from 'quark/fs';
import url from 'quark/path';
import * as uu from './uu';

var path = url.documents('benchmark.txt');

fs.writeFileSync(path, 'ABCDEFGHIJKMLN');

var i = 0;

uu.start();

for (var j = 0; j < 10000; j++) {
	fs.reader.readFileSync(path);
}

uu.log();

for (var j = 0; j < 10000; j++) {
	fs.reader.readFile(path).then(function(data) {
	}).catch(e=>console.log('err', ++i));
}

uu.log();
