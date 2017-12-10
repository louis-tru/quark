
var ts = 0;

function start(...args) {
	ts = new Date().valueOf();
	console.log('time: start', ...args);
}

function time(...args) {
	var ts2 = new Date().valueOf();
	console.log('time:', ts2 - ts, ...args);
	ts = ts2;
}

exports.start = start;
exports.time = time;
