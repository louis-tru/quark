
import * as http from 'ftr/http';

var i = 0;

for (var j = 0; j < 1000; j++) {
	http.get('https://github.com').then(function({data}) {
		console.log('ok', ++i, data.length);
	}).catch(e=>console.log('err', ++i));
}
