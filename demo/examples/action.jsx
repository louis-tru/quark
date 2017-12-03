import { 
	Div, Hybrid, Text, Button, Image, Indep, Clip,
} from 'ngui';
import HIGHLIGHTED_DOWN from 'ngui/event';
import Toolbar from 'ngui/nav';
import Mynavpage from './public';
import './review';

function view_code(evt) {
  evt.sender.topCtr.collection.push(review.vx, 1);
}

function highlighted(evt) {
	var img1 = evt.sender.topCtr.find('img1');
	var img2 = evt.sender.topCtr.find('img2');
	var speed = 1;
	if ( evt.status == HIGHLIGHTED_DOWN ) {
		speed = img1 === evt.sender ? 2 : 0.5;
	}
	img1.action.speed = speed;
	img2.action.speed = speed;
}

const toolbar_vx = (
  <Toolbar backgroundColor="#333">
    <Hybrid textAlign="center" width="full" height="full">
      <Button onClick=view_code>
        <Text class="toolbar_btn" textColor="#fff">\ue9ab</Text>
      </Button>
    </Hybrid>
  </Toolbar>
)

export const vx = (
  <Mynavpage 
    navbar.backgroundColor="#333"
    navbar.backTextColor="#fff" 
    navbar.titleTextColor="#fff"
    toolbar=toolbar_vx
    backgroundColor="#333"
    title="Action" source=resolve(__filename)>
    <Clip width="full" height="full">

	  	<Indep width=600 alignX="center" alignY="center" y=-15 opacity=0.5>
	    	<Image onHighlighted=highlighted id="img1" src=resolve('./gear0.png')
	    		marginLeft="auto" marginRight="auto" 
	    		y=56 width=600 origin="300 300"
	    		action=[
						{ rotateZ: 0, time:0, curve:'linear' }, 
						{ rotateZ: -360, time: 4000, curve:'linear' },
				  ]
	    		action.loop=1e8
	    		action.playing=1
	    	/>
	    	<Image onHighlighted=highlighted id="img2" src=resolve('./gear1.png')
	    		marginLeft="auto" 
	    		marginRight="auto"
	    		width=361 
	    		origin="180.5 180.5"
	    		action=[
						{ rotateZ: 22.5, time:0, curve:'linear' }, 
						{ rotateZ: 22.5 + 360, time: 2000, curve:'linear' },
					]
	    		action.loop=1e8
	    		action.playing=1
	    	/>
	  	</Indep>

	  </Clip>
  </Mynavpage>
)
