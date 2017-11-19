
import fs from 'fs';

var buf = fs.readFileSync('./test_node.js');

console.log(buf.toString());

for ( var i in this ) {
	console.log(i, this[i]);
}
