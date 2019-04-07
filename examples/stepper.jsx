import { Div, Text, CSS, atomPixel } from 'qgr';
import { Stepper } from 'qgr/stepper';
import { Mynavpage } from './public';

var resolve = require.resolve;

CSS({
	'.strpper_page': {
		width: 'full',
	},
	'.strpper_page .item': {
		width: 'full',
		borderBottom: `${atomPixel} #ccc`,
	},
	'.strpper_page .text': {
		width: '140!',
		margin: 13,
	},
})

function change_handle(evt) {
	var stepper = evt.sender;
	stepper.view.prev.value = stepper.value;
}

export const vx = (
	<Mynavpage title="Stepper" source=resolve(__filename)>
		<Div width="full" class="strpper_page">
			<Div class="item">
				<Text class="text" />
				<Stepper onChange=change_handle style={margin:10} value=10 />
			</Div>
			<Div class="item">
				<Text class="text" />
				<Stepper onChange=change_handle style={margin:10} max=10 min=5 value=6 />
			</Div>
			<Div class="item">
				<Text class="text" value="0" />
				<Stepper onChange=change_handle style={margin:10} step=0.1 />
			</Div>
		</Div>
	</Mynavpage>
)
