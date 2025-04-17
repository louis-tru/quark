
var fs = require('fs');
var path = require('path');
var variables = JSON.parse(fs.readFileSync('out/config.gypi', 'utf-8')).variables;

function touch_file(pathnames) {
	if ( !Array.isArray(pathnames)) {
		pathnames = [ pathnames ];
	}
	pathnames.forEach(function(pathname) {
		if ( !fs.existsSync(pathname) ) {
			fs.mkdirSync(path.dirname(pathname), {recursive:true});
			fs.writeFileSync(pathname, '');
		}
	});
}

function touch_files() {
	touch_file([
		'out/native-inl-js.cc',
		'out/native-lib-js.cc',
		'out/native-font.cc',
	]);
	if (variables.os == 'android' && (variables.debug || variables.without_visibility_hidden)) {
		touch_file([
			`${variables.output}/obj.target/libquark_deps_test.so`,
		]);
	}
}

touch_files();