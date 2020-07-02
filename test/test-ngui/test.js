
import 'ftr/util';

function json_stringify(value, whitelist) {

	var m = { '\b': '\\b', '\t': '\\t', '\n': '\\n', '\f': '\\f', '\r': '\\r', '"': '\\"', '\\': '\\\\' };
	var a, i, k, l, v, r = /["\\\x00-\x1f\x7f-\x9f']/g;

	switch (typeof value) {
		case 'string':

			return (r.test(value) ? '"' + value.replace(r, function(a) {
				var c = m[a];
				if (c)
					return c;

				c = a.charCodeAt();

				return '\\u00' + Math.floor(c / 16).toString(16) + (c % 16).toString(16);
			}) 
				+ '"' : '"' + value + '"');

		case 'number':
			return isFinite(value) ? String(value) : NaN;

		case 'boolean':
			return value.toString();

		case 'null':
			return String(value);

		case 'object':
			if (!value)
				return 'null';
			if (typeof value.getDate === 'function') {

				var year = value.getUTCFullYear();
				var month = value.getUTCMonth() + 1;
				var date = value.getUTCDate();
				var hours = value.getUTCHours();
				var minutes = value.getUTCMinutes();
				var seconds = value.getUTCSeconds();
				var milliseconds = value.getUTCMilliseconds();

				return year + '-' +
					(month < 10 ? '0' : '') + month + '-' +
					(date < 10 ? '0' : '') + date + 'T' +
					(hours < 10 ? '0' : '') + hours + ':' +
					(minutes < 10 ? '0' : '') + minutes + ':' +
					(seconds < 10 ? '0' : '') + seconds + '.' +
					milliseconds + 'Z';
			}

			a = [];
			if (typeof value.length === 'number' && !(value.propertyIsEnumerable('length'))) {

				l = value.length;

				for (i = 0; i < l; i += 1)
					a.push(json_stringify(value[i], whitelist) || 'null');

				return '[' + a.join(',') + ']';
			}

			if (whitelist) {
				l = whitelist.length;
				for (i = 0; i < l; i += 1) {
					k = whitelist[i];
					if (typeof k === 'string') {
						v = json_stringify(value[k], whitelist);
						if (v)
							a.push(json_stringify(k) + ':' + v);
					}
				}
			}
			else {

				for (k in value) {
					if (typeof k === 'string') {
						v = json_stringify(value[k], whitelist);
						if (v)
							a.push(json_stringify(k) + ':' + v);
					}
				}

			}
			return '{' + a.join(',') + '}';

		case 'function': 
			return 'Function';
	}
};

export function LOG(...args) {
	console.log(...args);
}

export function P(self, name, value) {
	if ( arguments.length > 2 ) { // set
		console.log('Property-Set', name, ':', (self[name] = value));
	} else {
		console.log('Property', name, ':', self[name]);
	}
}

export function M(self, name, args = [], v) {
	var r = self[name]( ...args );
	var vv = '';
	var argc = arguments.length;

	if ( argc > 3 ) {
		if (typeof v == 'function') {
			vv = v(...[r]) ? 'ok': 'no';
		} else {
			vv = v == r ? 'ok': 'no';
		}
	}
	console.log( 'Method', name + '()', ':', r, vv);

	return r;
}

export function AM(self, name, args = [], v) {
	var argc = arguments.length;

	return new Promise(function(resolve, reject) {
		var async = false;
		var r;

		function ok(sync, ...args) {
			var vv = '';
			if ( argc > 3 ) {
				var v_args = args;
				if (sync) v_args = [r];
				if (typeof v == 'function') {
					vv = v(...v_args) ? 'ok': 'no';
				} else {
					vv = v == v_args[0] ? 'ok': 'no';
				}
			}
			console.log( 'Method', name + '()', ':', r, ...args, vv);
			resolve(args[0]);
		}
		
		args = args.map(function(e) {
			if (typeof e != 'function') {
				return e;
			}
			async = true;
			return function(...args) {
				if ( e(...args) )
					ok(0, ...args);
			}.catch(reject);
		});

		r = self[name]( ...args );

		if ( !async ) {
			ok(1);
		}
	});
}

export function VM(self, name, args = [], value = true) {
	if (typeof value == 'function') {
		console.log(value(self[name](...args)) ? 'ok': 'no');
	} else {
		console.log(self[name](...args) == value ? 'ok': 'no');
	}
}

export function VP(self, name, value = true) {
	if (typeof value == 'function') {
		console.log(value(self[name]) ? 'ok': 'no');
	} else {
		console.log(self[name] == value ? 'ok': 'no');
	}
}

export function CL() {
	var i = 0;

	function clear() {
		util.garbageCollection();
		i++;
		if ( i < 3 ) {
			setTimeout(clear, 1000);
		}
	}

	clear();
}

export function CA(func) {
	func().catch(function(err) {
		LOG('Error:');
		LOG(err.message);
		if (err.stack) {
			LOG(err.stack);
		}
	});
}

exports.CALL_ASYNC = CA

