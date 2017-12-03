import { Div, Button, Input } from 'ngui';
import 'ngui/fs';
import 'ngui/url';
import alert from 'ngui/dialog';
import Mynavpage from './public';

const filename = url.documents('test.txt');

function WriteFile(evt) {
	fs.writeFile(filename, evt.sender.topCtr.find('input').value, function(err) {
		if (err)
			alert(err.message);
		else
			alert('Write file OK.');
	});
}

function WriteFileSync(evt) {
	try {
		var txt = evt.sender.topCtr.find('input').value;
		// console.log('WriteFileSync', txt);
		fs.writeFileSync(filename, txt);
		alert('Write file OK.');
	} catch (err) {
		alert(err.message);
	}
}

function ReadFile(evt) {
	fs.readFile(filename, function(err, buf) {
		if (err) 
			alert(err.message);
		else 
			alert(buf.toString('utf-8'));
	});
}

function Remove(evt) {
	try {
		fs.removeSyncR(filename);
		alert('Remove file OK.');
	} catch (err) {
		alert(err.message);
	}
}

function keyenter(evt) {
	evt.sender.blur();
}

export const vx = (
  <Mynavpage title="File System" source=resolve(__filename)>
    <Div width="full">
    	<Input class="input" id="input" 
    		placeholder="Please enter write content.."
    		value="Hello."
    		returnType="done" onKeyenter=keyenter />
      <Button class="long_btn" onClick=WriteFile>WriteFile</Button>
      <Button class="long_btn" onClick=WriteFileSync>WriteFileSync</Button>
      <Button class="long_btn" onClick=ReadFile>ReadFile</Button>
      <Button class="long_btn" onClick=ReadFile>ReadFileSync</Button>
      <Button class="long_btn" onClick=Remove>Remove</Button>
    </Div>
  </Mynavpage>
)