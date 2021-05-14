
import * as fs from 'flare/fs';
import url from 'flare/path';
import * as reader from 'flare/reader';
import * as uu from './uu';

var path = url.documents('benchmark.txt');

fs.writeFileSync(path, 'ABCDEFGHIJKMLN');

var i = 0;

uu.start();

for (var j = 0; j < 10000; j++) {
	reader.readFileSync(path);
}

uu.log();

for (var j = 0; j < 10000; j++) {
	reader.readFile(path).then(function(data) {
	}).catch(e=>console.log('err', ++i));
}

uu.log();
