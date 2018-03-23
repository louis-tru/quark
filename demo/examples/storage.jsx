import { Div, Button, Input } from 'ngui';
import 'ngui/storage';
import { alert } from 'ngui/dialog';
import { Mynavpage } from './public';

const key = 'test';

function keyenter(evt) {
	evt.sender.blur();
}

function Get(evt) {
  var val = storage.get(key);
  if ( val ) {
    alert(storage.get(key));
  } else {
    alert('No local storage data！');
  }
}

function Set(evt) {
	storage.set(key, evt.sender.topCtr.find('input').value);
  alert('Save local data OK.');
}

function Del(evt) {
	storage.del(key);
  alert('Delete local data OK.');
}

function Clear(evt) {
	storage.clear(key);
  alert('Delete All local data OK.');
}

export const vx = (
  <Mynavpage title="Local Storage" source=resolve(__filename)>
    <Div width="full">
    	<Input class="input" id="input" 
    		placeholder="Please enter value .." 
    		value="Hello."
    		returnType="done" onKeyEnter=keyenter />
      <Button class="long_btn" onClick=Get>Get</Button>
      <Button class="long_btn" onClick=Set>Set</Button>
      <Button class="long_btn" onClick=Del>Del</Button>
      <Button class="long_btn" onClick=Clear>Clear</Button>
    </Div>
  </Mynavpage>
)