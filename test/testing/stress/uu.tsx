
import util from 'quark/util';
import * as os from 'quark/os';
import quark from 'quark';
import { _CVD, ViewController, linkAcc } from 'quark';

let ts = 0;
let fsp: FSP;

class FSP extends ViewController {
	@linkAcc value = '';
	render() {
		return (
			<matrix align="leftBottom" x={5} y={-5}>
				<text textColor="#f00" value={this.value} />
			</matrix>
		);
	}
	up_fsp() {
		let fsp_value = this.metaView.window.fsp;
		let cpu_usage = os.cpuUsage();
		this.value = 'FSP: ' + fsp_value + ', CPU: ' + (cpu_usage * 100).toFixed(0) + '%';
		setTimeout(()=>this.up_fsp(), 1000);
	}
}

export function show_fsp() {
	util.assert(!fsp);
	util.assert(quark.app);

	fsp = quark.app.activeWindow!.render(<FSP />) as FSP;
	fsp.up_fsp();
};

export function start(...args: any[]) {
	ts = new Date().valueOf();
	console.log('start:', ...args);
}

export function log(...args: any[]) {
	let ts2 = new Date().valueOf();
	console.log('time:', ts2 - ts, ...args);
	ts = ts2;
}
