import { Div, Button, Text, Input, Textarea, ngui } from 'ngui';
import { Mynavpage } from './public';

function start_input(evt) {
  evt.sender.topCtr.find('input1').focus();
}

function end_input(evt) {
  ngui.app.focusView.blur();
}

export const vx = (
  <Mynavpage title="Input" source=resolve(__filename)>
    <Div width="full">
      <Text margin=10 textBackgroundColor="#000" textColor="#fff">Examples Input</Text>
      
      <Input id="input0" margin=10 
        width="full" 
        height=30  
        backgroundColor="#eee"
        type="phone"
        returnType="next"
        borderRadius=8 placeholder="Please enter.." />

      <Input id="input1" margin=10 
        width="full" 
        textColor="#fff"
        backgroundColor="#000"
        height=30  
        border="0 #f00" 
        borderRadius=0
        type="decimal"
        textAlign="center" 
        placeholder="Please enter.." value="Hello" />
      
      <Textarea margin=10 
        width="full" 
        height=120 
        textColor="#000"
        border="0 #aaa" 
        backgroundColor="#eee"
        borderRadius=8
        returnType="next"
        placeholder="Please enter.."
        textSize=14
        textAlign="center" />
        
      <Button class="long_btn" onClick=end_input>Done</Button>
      <Button class="long_btn" onClick=start_input>Input</Button>
        
    </Div>
  </Mynavpage>
)