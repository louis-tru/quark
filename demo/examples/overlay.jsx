import { Div, Text, CSS, atomPixel, Button, Indep, New } from 'ngui';
import { Navbutton, Mynavpage } from './public';
import Overlay from 'ngui/overlay';

function show_overlay(evt) {
  New(
    <Overlay>
      <Div>
        <Navbutton>Menu A</Navbutton>
        <Navbutton>Menu B------C</Navbutton>
        <Navbutton>Menu C</Navbutton>
        <Navbutton view.borderWidth=0>Menu D</Navbutton>
      </Div>
    </Overlay>
  ).showOverlayWithView(evt.sender);
}

function show_overlay2(evt) {
  var com = New(
    <Overlay>
      <Div>
        <Navbutton>Hello.</Navbutton>
        <Navbutton>Who are you going to?</Navbutton>
        <Navbutton view.borderWidth=0>Do I know you?</Navbutton>
      </Div>
    </Overlay>
  );
  com.priority = 'left';
  com.showOverlayWithView(evt.sender);
}

function show_overlay3(evt) {
  var com = New(
    <Overlay>
      <Div>
        <Navbutton view.textColor="#fff">Hello.</Navbutton>
        <Navbutton view.textColor="#fff">Who are you going to?</Navbutton>
        <Navbutton view.textColor="#fff">Do I know you?</Navbutton>
        <Navbutton view.textColor="#fff" view.borderWidth=0>What country are you from?</Navbutton>
      </Div>
    </Overlay>
  );
  com.priority = 'left';
  com.backgroundColor = '#000';
  com.showOverlayWithView(evt.sender);
}

export const vx = (
  <Mynavpage title="Overlay" source=resolve(__filename)>
    <Div width="full" height="full">
      <Indep alignY="top" width="full">
        <Button class="long_btn" onClick=show_overlay> Show Overlay </Button>
      </Indep>
      <Indep alignY="bottom" y=-10 width="full">
        <Button class="long_btn" onClick=show_overlay> Show Overlay </Button>
      </Indep>
      <Indep alignY="center">
        <Button class="long_btn" onClick=show_overlay2> Show Overlay </Button>
      </Indep>
      <Indep alignY="center" alignX="right">
        <Button class="long_btn" onClick=show_overlay3> Show Overlay </Button>
      </Indep>
    </Div>
  </Mynavpage>
)