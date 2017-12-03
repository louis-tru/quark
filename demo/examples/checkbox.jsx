import { Div, Text, CSS, atomPixel } from 'ngui';
import { Switch, Checkbox } from 'ngui/checkbox';
import Mynavpage from './public';

CSS({
  '.checkbox_page': {
    width: 'full',
  },
  '.checkbox_page .item': {
    width: 'full',
    border_bottom: `${atomPixel} #ccc`,
  },
  '.checkbox_page .text': {
    width: '100!',
    margin: 13,
  },
})

function change_handle(evt) {
  var checkbox = evt.sender;
  var str = checkbox.selected ? 'YES' : 'NO';
  str += checkbox.disable ? ',Disable' : '';
  checkbox.view.prev.value = str;
}

export const vx = (
  <Mynavpage title="Checkbox" source=resolve(__filename)>
    <Div width="full" class="checkbox_page">
      <Div class="item">
        <Text class="text" value="YES" />
        <Switch onChange=change_handle style={margin:10} selected=1 />
      </Div>
      <Div class="item">
        <Text class="text" value="NO,Disable" />
        <Switch onChange=change_handle style={margin:10} disable=1 />
      </Div>
      <Div class="item">
        <Text class="text" value="NO" />
        <Switch onChange=change_handle style={margin:10} />
      </Div>
      <Div class="item">
        <Text class="text" value="YES" />
        <Checkbox onChange=change_handle style={margin:13} selected=1 />
      </Div>
      <Div class="item">
        <Text class="text" value="YES,Disable" />
        <Checkbox onChange=change_handle style={margin:13} disable=1 selected=1 />
      </Div>
      <Div class="item">
        <Text class="text" value="NO" />
        <Checkbox onChange=change_handle style={margin:13} />
      </Div>
    </Div>
  </Mynavpage>
)
