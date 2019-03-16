import { Div, Button, Input } from 'qgr';
import 'qgr/fs';
import 'qgr/path';
import { alert } from 'qgr/dialog';
import { Mynavpage } from './public';

const filename = path.documents('test.txt');

fs.mkdirpSync(path.dirname(filename));

function WriteFile(evt) {
	console.log('------------', filename);
	fs.writeFile(filename, evt.sender.owner.find('input').value, function() {
		alert('Write file OK.');
	}.catch(err=>{
		alert(err.message + ', ' + err.code);
	}));
}

function WriteFileSync(evt) {
	try {
		var txt = evt.sender.owner.find('input').value;
		var r = fs.writeFileSync(filename, txt);
		console.log(r);
		alert('Write file OK.');
	} catch (err) {
		alert(err.message + ', ' + err.code);
	}
}

function ReadFile(evt) {
	console.log('------------', filename);
	fs.readFile(filename, function(buf) {
		alert(buf.toString('utf-8'));
	}.catch(err=>{
		alert(err.message + ', ' + err.code);
	}));
}

function Remove(evt) {
	try {
		var a = fs.removerSync(filename);
		alert('Remove file OK. ' + a);
	} catch (err) {
		alert(err.message + ', ' + err.code);
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
				returnType="done" onKeyEnter=keyenter />
			<Button class="long_btn" onClick=WriteFile>WriteFile</Button>
			<Button class="long_btn" onClick=WriteFileSync>WriteFileSync</Button>
			<Button class="long_btn" onClick=ReadFile>ReadFile</Button>
			<Button class="long_btn" onClick=ReadFile>ReadFileSync</Button>
			<Button class="long_btn" onClick=Remove>Remove</Button>
		</Div>
	</Mynavpage>
)