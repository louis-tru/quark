import { Scroll, Div, Clip, Text } from 'ngui';
import { Navbutton, Mynavpage } from './public';
import './components';
import './input';
import './icons';
import './media';
import './action';
import './fs';
import './http';
import './zlib';
import './storage';

export const vx = (

  <Mynavpage title="Examples" source=resolve(__filename)>

    <Scroll width="full" height="full" bounceLock=0>

      <Text class="category_title">GUI.</Text>
      <Clip class="category">
        <Navbutton next=components.vx id="btn0">Components</Navbutton>
        <Navbutton next=media.vx>Multi-Media</Navbutton>
        <Navbutton next=input.vx>Input</Navbutton>
        <Navbutton next=icons.vx>Icons</Navbutton>
        <Navbutton next=action.vx view.borderWidth=0>Action</Navbutton>
      </Clip>
      
      <Text class="category_title">Basic util.</Text>
      <Clip class="category">
        <Navbutton next=fs.vx>File System</Navbutton>
        <Navbutton next=http.vx>Http</Navbutton>
        <!--Navbutton next=zlib.vx>Zlib</Navbutton-->
        <Navbutton next=storage.vx view.borderWidth=0>Local Storage</Navbutton>
      </Clip>

      <Div height=15 width="full" />
    </Scroll>

  </Mynavpage>

)

