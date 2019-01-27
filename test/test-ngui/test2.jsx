
import { GUIApplication, Root, Indep, Text, Div } from 'shark';

new GUIApplication({ multisample: 2 }).start(
	<Root>
		<Div backgroundColor="#aaa" width="100%" height="100%">
			<Text textColor="#f00" >ABCDEFG你好</Text>
		</Div>
	</Root>
);
