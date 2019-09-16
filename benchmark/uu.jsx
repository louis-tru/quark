
import 'ngui/http';
import 'ngui/util';
import 'ngui/sys';
import { Indep, ngui, ViewController, render, Text } from 'ngui';

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

class FSP extends ViewController {

	render() {
		var { fsp_value = 0, cpu_usage = 0 } = this.modle;
		var value = 'FSP: ' + fsp_value + ', CPU: ' + (cpu_usage * 100).toFixed(0) + '%';
		return (
			<Indep alignY="bottom" x=5 y=-5>
				<Text textColor="#f00" value=value />
			</Indep>
		);
	}

	up_fsp() {
		var fsp_value = ngui.displayPort.fsp();
		var cpu_usage = sys.cpuUsage();
		this.modle = { fsp_value, cpu_usage };
		setTimeout(e=>this.up_fsp(), 1000);
	}

}

var fsp;

exports.show_fsp = function() {
	util.assert(!fsp);
	util.assert(ngui.app);

	function show_fsp_ok() {
		if (ngui.root) {
			fsp = render(<FSP />, ngui.root);
			fsp.up_fsp();
		}
	}

	if (ngui.app.isLoaded) {
		show_fsp_ok.setTimeout(1000);
	} else {
		ngui.app.onLoad.on(e=>show_fsp_ok.setTimeout(1000));
	}
};

exports.start = start;
exports.log = log;
