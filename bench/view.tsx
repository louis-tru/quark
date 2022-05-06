
import { Application, ViewController, Root, Div, Scroll, default as noug, _CVD } from 'noug';
import { Color } from 'noug/value';
import util from 'noug/util';
import * as uu from './uu';

const {random} = util

noug.css({
	'.item': {
		height: 40,
		width: '25%',
	},
})

class RootViewController extends ViewController {

	triggerMounted() {
		super.triggerMounted();

		uu.start();

		noug.render<Scroll>(
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
