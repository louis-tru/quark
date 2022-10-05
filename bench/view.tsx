
import { Application, ViewController, Root, Div, Scroll, default as quark, _CVD } from 'quark';
import { Color } from 'quark/value';
import util from 'quark/util';
import * as uu from './uu';

const {random} = util

quark.css({
	'.item': {
		height: 40,
		width: '25%',
	},
})

class RootViewController extends ViewController {

	triggerMounted() {
		super.triggerMounted();

		uu.start();

		quark.render<Scroll>(
			<Scroll width="full" height="full">
				{
					Array.from({ length: 10000 }, ()=>{
						var color = new Color(random(0, 255), 
							random(0, 255), random(0, 255), 255);
						return <Div backgroundColor={color} class="item" />;
					})
				}
			</Scroll>
		).appendTo(this.domAs());
	
		uu.log();

	}
}

new Application({ multisample: 2 }).start(
	<RootViewController><Root backgroundColor="#000" /></RootViewController>
);

uu.show_fsp();
