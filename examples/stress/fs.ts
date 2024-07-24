
import * as fs from 'quark/fs';
import url from 'quark/path';
import * as uu from './uu';

var path = url.documents('benchmark.txt');

fs.writeFileSync(path, 'ABCDEFGHIJKMLN');

var i = 0;

uu.start();

for (var j = 0; j < 1000; j++) {
	fs.copyrSync(path, path + '.' + j);
}

uu.log();

for (var j = 0; j < 1000; j++) {
	fs.copyr(path, path + '.' + j).then(function(){
		//
	}).catch(function(err) {
		console.log('copy err', ++i);
	});
}

var stat = fs.statSync(path);

for (var j = 0; j < 1000; j++) {
	fs.chmodr(path + '.' + j, stat.mode()).then(function() {
		//
	}).catch(e=>console.log('chmodr err', ++i));
}

for (var j = 0; j < 1000; j++) {
	fs.chownr(path + '.' + j, stat.owner(), stat.group()).then(function() {
		//
	}).catch(e=>console.log('chownr err', ++i));
}

for (var j = 0; j < 1000; j++) {
	fs.isFile(path + '.' + j).then(function(ok) {
		// console.log('isFile ok', ok);
	}).catch(e=>console.log('isFile err', ++i));
}

uu.log();
