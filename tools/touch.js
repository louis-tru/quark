
var fs = require('fs');
var path = require('path');

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

function touch_files(variables) {
	touch_file([
		'out/native-inl-js.cc',
		'out/native-ext-js.cc',
		'out/native-lib-js.cc',
		'out/native-font.cc',
		'out/native-glsl.cc',
	]);

	if (['mac','ios'].indexOf(variables.os) == -1) {
		touch_file([
			`${variables.output}/obj.target/libquark.so`,
		]);
	}
	if (variables.os == 'android' && (variables.debug || variables.without_visibility_hidden)) {
		touch_file([
			`${variables.output}/obj.target/libquark_deps_test.so`,
		]);
	}
}

var variables = JSON.parse(fs.readFileSync('out/config.gypi', 'utf-8')).variables;

touch_files(variables);