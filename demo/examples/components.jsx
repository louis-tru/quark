import { Scroll, Clip, Text, ngui } from 'ngui';
import { Navbutton, Mynavpage } from './public';
import './checkbox';
import './overlay';
import './stepper';
import './nav';
import './dialog';
import './list';

export const vx = (
  <Mynavpage title="Components" source=resolve(__filename)>
    <Scroll width="full" height="full" bounceLock=0>
    
      <Text class="category_title">Components.</Text>
      <Clip class="category">
        <Navbutton next=nav.vx>Nav</Navbutton>
        <Navbutton next=checkbox.vx>Checkbox</Navbutton>
        <Navbutton next=stepper.vx>Stepper</Navbutton>
        <Navbutton next=overlay.vx>Overlay</Navbutton>
        <Navbutton next=dialog.vx>Dialog</Navbutton>
        <Navbutton next=list.vx view.borderWidth=0>List</Navbutton>
      </Clip>
      
    </Scroll>
  </Mynavpage>
);
