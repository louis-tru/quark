
import 'ngui/fs';
import 'ngui/url';
import 'ngui/reader';
import './print';

var path = url.documents('benchmark.txt');

fs.writeFileSync(path, 'ABCDEFGHIJKMLN');

var i = 0;

print.start();

for (var j = 0; j < 10000; j++) {
	reader.readFileSync(path);
}

print.time();

for (var j = 0; j < 10000; j++) {
	reader.readFile(path, function(bf) {

	}.catch(e=>console.log('err', ++i)));
}

print.time();
