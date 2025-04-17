
var fs = require('encark/fs');
var variables = JSON.parse(fs.readFileSync('out/config.gypi', 'utf-8')).variables;

function cp_libquarkso() {
	var os = variables.os;
	if (['ios'].indexOf(os) == -1) {
		var output = variables.output;
		var product = `${variables.output}/../qkmake/product/${os}`;
		var home = `${product}/${variables.arch}`;
		if (os=='android') {
			fs.cp_sync(`${output}/libquark.so`,
				`${product}/jniLibs/${variables.android_abi}/libquark.so`, { replace: true });
		} else if (os == 'linux') {
			fs.cp_sync(`${output}/libquark.so`, `${home}/libquark.so`, { replace: true });
			fs.cp_sync(`${output}/quark`, `${home}/quark`, { replace: true });
		} else if (os == 'mac') {
			// fs.cp_sync(`${output}/libquark.so`, `${home}/quark`, { replace: true });
			fs.cp_sync(`${output}/quark`, `${home}/quark`, { replace: true });
		}
		fs.chmod_r(product, 0o755);
	}
}

cp_libquarkso();