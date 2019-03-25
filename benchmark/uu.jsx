
import 'qgr/http';
import 'qgr/util';
import 'qgr/sys';
import { Indep, qgr, New, Text } from 'qgr';

var ts = 0;

function start(...args) {
	ts = new Date().valueOf();
	console.log('start:', ...args);
}

function log(...args) {
	var ts2 = new Date().valueOf();
	console.log('time:', ts2 - ts, ...args);
	ts = ts2;
}

function show_fsp_ok() {

	var displayPort = qgr.displayPort;
	var fsp = null;
	var priv_fsp_value = 0;
	var priv_cpu_usage = 0;

	function up_fsp() {
		var fsp_value = displayPort.fsp();
		var cpu_usage = sys.cpuUsage();
		if (priv_fsp_value != fsp_value || priv_cpu_usage != cpu_usage) {
			fsp.value = 'FSP: ' + fsp_value + ', CPU: ' + (cpu_usage * 100).toFixed(0);
			priv_fsp_value = fsp_value;
		}
		setTimeout(up_fsp, 1000);
	}

	setTimeout(function() {
		var root = qgr.root;
		if (root) {
			fsp = New(
				<Indep alignY="bottom" x=5 y=-5>
					<Text textColor="#f00" />
				</Indep>
			, root).first;

			up_fsp();
		}
	}, 1000);

}

function show_fsp() {
	var app = qgr.app;
	util.assert(app);

	if (app.isLoad) {
		show_fsp_ok();
	} else {
		app.onLoad.on(show_fsp_ok);
	}
}

exports.start = start;
exports.log = log;
exports.show_fsp = show_fsp;
