
import 'ngui/http';

var i = 0;

for (var j = 0; j < 1000; j++) {
	http.get('https://github.com', function(bf) {
		console.log('ok', ++i, bf.length);
	}.catch(e=>console.log('err', ++i)));
}
