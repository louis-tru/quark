import { Div, Button } from 'qgr';
import { Mynavpage } from './public';

var resolve = require.resolve;

export const vx = (
	<Mynavpage title="Zlib" source=resolve(__filename)>
		<Div width="full">
			<Button class="long_btn">OK</Button>
		</Div>
	</Mynavpage>
)