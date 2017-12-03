import { Div, Button, TextNode } from 'ngui';
import Mynavpage from './public';
import 'ngui/dialog';

function alert() {
  dialog.alert('Hello alert.');
}

function confirm() {
  dialog.confirm('Hello Confirm.', (ok)=>{
    if ( ok ) dialog.alert('OK');
  });
}

function prompt() {
  dialog.prompt('Hello Prompt.', (ok, text)=>{
    if ( ok ) {
      dialog.alert(text);
    }
  });
}

function custom() {
  dialog.show('蓝牙已关闭', 
  'CarPlay将只能通过USB使用。您希望同时启用无线CarPlay吗？', 
  [<TextNode textStyle='bold'>仅USB</TextNode>, '无线蓝牙'], (num)=>{
    if ( num == 0 ) {
      dialog.alert('仅USB');
    } else {
      dialog.alert('无线蓝牙');
    }
  });
}

export const vx = (
  <Mynavpage title="Dialog" source=resolve(__filename)>
    <Div width="full">
      <Button class="long_btn" onClick=alert>Alert</Button>
      <Button class="long_btn" onClick=confirm>Confirm</Button>
      <Button class="long_btn" onClick=prompt>Prompt</Button>
      <Button class="long_btn" onClick=custom>Custom</Button>
    </Div>
  </Mynavpage>
)