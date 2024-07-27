
import { _CVD, Application, Window } from 'quark';
import * as types from 'quark/types';
import util from 'quark/util';
import * as css from 'quark/css';
import * as uu from './uu';

const random = util.random;
const test_count = 100000;

const app = new Application();
const win = new Window({
	title: 'tests css',
	frame: types.newRect(0,0,500,500),
}).activate();

const w = win.size.x;
const h = win.size.y;
const csss: Dict<css.StyleSheet> = {
	'.top': {
		backgroundColor: '#000',
	},
};

for (var i = 0; i < test_count; i++) {
	var s = random(20, 30);
	var s2 = s / 2;
	csss['.root .css_' + i] = {
		backgroundColor: new types.Color(random(0, 255), random(0, 255), random(0, 255), 255),
		width: s,
		height: s,
		x: random(0, w + s) - s2,
		y: random(0, h + s) - s2,
	};
}

uu.start();
css.createCss(csss);
uu.log();

win.render(
	<box width="match" height="match" class="top">
	{
		Array.from({length:test_count}, (_, i)=>{
			return <matrix key={i} class={'css_' + i} />;
		})
	}
	</box>
);

uu.log();