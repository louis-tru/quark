
import 'langou/fs';
import * as url from 'langou/path';
import './uu';

var path = url.documents('benchmark.txt');

fs.writeFileSync(path, 'ABCDEFGHIJKMLN');

var i = 0;

uu.start();

for (var j = 0; j < 1000; j++) {
	fs.copyrSync(path, path + '.' + j);
}

uu.log();

for (var j = 0; j < 1000; j++) {
	fs.copyr(path, path + '.' + j, function(err) {
		if (err) {
			console.log('copy err', ++i);
		}
	});
}

var stat = fs.statSync(path);

for (var j = 0; j < 1000; j++) {
	fs.chmodr(path + '.' + j, stat.mode(), function() {
	}.catch(e=>console.log('chmodr err', ++i)));
}

for (var j = 0; j < 1000; j++) {
	fs.chownr(path + '.' + j, stat.owner(), stat.group(), function() {
	}.catch(e=>console.log('chownr err', ++i)));
}

for (var j = 0; j < 1000; j++) {
	fs.isFile(path + '.' + j, function(ok) {
		// console.log('isFile ok', ok);
	}.catch(e=>console.log('isFile err', ++i)));
}

uu.log();
