
import { Application, ViewController, Root, Indep, default as quark, _CVD } from 'quark';
import { Color } from 'quark/value';
import util from 'quark/util';
import * as uu from './uu';

const random = util.random;

class RootViewController extends ViewController {
	render() {
		var w = quark.displayPort.width;
		var h = quark.displayPort.height;
		console.log(w,h);

		return (
			<Root backgroundColor="#000">
				{
					Array.from({ length: 10000 }).map((e,i)=>{
						var color = new Color(random(0, 255), random(0, 255), random(0, 255), 255);
						var s = random(20, 30);
						var s2 = s / 2;
						var x = random(0, w + s) - s2;
						var y = random(0, h + s) - s2;
						return (
							<Indep originX={s2} 
								originY={s2}
								backgroundColor={color} 
								width={s} 
								height={s} 
								x={x} 
								y={y} 
								action={{
									keyframe: [
										{ rotateZ: 0, time:0, curve:'linear' }, 
										{ rotateZ: 360 * (random(0, 1) ? -1 : 1),
											time: random(1000, 4000), curve:'linear' },
									],
									loop: 1e8,
									playing: 1,
								}}
							/>
						);
					})
				}
			</Root>
		);
	}
}

new Application({ multisample: 4, title: 'Quark benchmark' }).start(<RootViewController />);

uu.show_fsp();
