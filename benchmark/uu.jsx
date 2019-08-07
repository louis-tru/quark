
import 'langou/http';
import 'langou/util';
import 'langou/sys';
import { Indep, langou, ViewController, render, Text } from 'langou';

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
	priv_fsp_value = 0;
	priv_cpu_usage = 0;

	render() {
		return (
			<Indep alignY="bottom" x=5 y=-5>
				<Text textColor="#f00" value=(this.modle.fsp || '') />
			</Indep>
		);
	}

	up_fsp() {
		var fsp_value = langou.displayPort.fsp();
		var cpu_usage = sys.cpuUsage();
		if (this.priv_fsp_value != fsp_value || this.priv_cpu_usage != cpu_usage) {
			this.priv_fsp_value = fsp_value;
			this.priv_cpu_usage = cpu_usage;
			this.modle = { fsp : 'FSP: ' + fsp_value + ', CPU: ' + (cpu_usage * 100).toFixed(0) };
		}
		setTimeout(e=>this.up_fsp(), 1000);
	}

}

var fsp;

exports.show_fsp = function() {
	util.assert(!fsp);
	util.assert(langou.app);

	function show_fsp_ok() {
		if (langou.root) {
			fsp = render(<FSP />, langou.root);
			fsp.up_fsp();
		}
	}

	if (langou.app.isLoaded) {
		show_fsp_ok.setTimeout(1000);
	} else {
		langou.app.onLoad.on(e=>show_fsp_ok.setTimeout(1000));
	}
};

exports.start = start;
exports.log = log;
