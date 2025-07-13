
import { Jsx, Application, Window, createCss } from 'quark';
import * as types from 'quark/types';
import util from 'quark/util';
import * as uu from './uu';

const {random} = util;

const app = new Application();
const win = new Window({
	title: 'tests view',
	frame: types.newRect(0,0,500,500),
}).activate();

createCss({
	'.item': {
		height: 40,
		width: '25%',
	},
})

uu.start();

win.render(
	<scroll width="match" height="match">
	{
		Array.from({ length: 10000 }, ()=>{
			return <box backgroundColor={new types.Color(
				random(0, 255),
				random(0, 255),
				random(0, 255), 255
			)} class="item" />;
		})
	}
	</scroll>
);

uu.log();
uu.show_fsp();
