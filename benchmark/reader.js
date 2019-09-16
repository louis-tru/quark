
import 'ngui/fs';
import * as url from 'ngui/path';
import 'ngui/reader';
import './uu';

var path = url.documents('benchmark.txt');

fs.writeFileSync(path, 'ABCDEFGHIJKMLN');

var i = 0;

uu.start();

for (var j = 0; j < 10000; j++) {
	reader.readFileSync(path);
}

uu.log();

for (var j = 0; j < 10000; j++) {
	reader.readFile(path, function(bf) {
	}.catch(e=>console.log('err', ++i)));
}

uu.log();
