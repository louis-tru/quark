
import { Application, Window, Jsx } from 'quark';
import * as types from 'quark/types';
import util from 'quark/util';
import * as uu from './uu';

const random = util.random;
const app = new Application();
const win = new Window({
	title: 'tests action',
	frame: types.newRect(0,0,500,500),
}).activate();

const w = win.size.x, h = win.size.y;
console.log(w,h);

win.render(
	<box width="match" height="match">
	{
		Array.from({length:10000}).map((e,i)=>{
			let color = new types.Color(random(0, 255), random(0, 255), random(0, 255), 255);
			let s = random(20, 30);
			let s2 = s / 2;
			let x = random(0, w + s) - s2;
			let y = random(0, h + s) - s2;
			return (
				<matrix
					key={i}
					originX={s2}
					originY={s2}
					backgroundColor={color}
					width={s}
					height={s}
					x={x}
					y={y}
					action={{
						keyframe: [
							{ curve: 'linear', rotateZ: 0 },
							{ curve: 'linear', rotateZ: 360 * (random(0, 1) ? -1 : 1), time: random(1000, 4000) },
						],
						loop: 1e8,
						playing: true,
					}}
				/>
			);
		})
	}
	</box>
)

uu.show_fsp()
