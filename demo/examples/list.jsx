import { Div, Button, CSS, Input, Text, atomPixel } from 'ngui';
import { List } from 'ngui/list';
import { Mynavpage } from './public';

function add(evt) {
	var text = evt.sender.topCtr.find('input').value;
	evt.sender.topCtr.find('list').push({ text: text });
}

function remove(evt) {
	evt.sender.topCtr.find('list').pop();
}

function keyenter(evt) {
	evt.sender.blur();
}

export const vx = (
  <Mynavpage title="List" source=resolve(__filename)>
    <Div width="full">
    	<Input id="input" class="input" 
    		value="Hello." returnType="done" onKeyEnter=keyenter />
      <Button class="long_btn" onClick=add>Add</Button>
      <Button class="long_btn" onClick=remove>Remove</Button>

      <List id="list">
      	<Div margin=10 width="full">
      		<Text margin=4 width="full" 
            borderBottom=`${atomPixel} #aaa`>%{$.$index + 1 + ': ' + $.text}</Text>
      	</Div>
      </List>

    </Div>
  </Mynavpage>
)