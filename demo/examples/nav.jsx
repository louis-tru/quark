import { 
  Div, Indep, Button, Text, Hybrid
} from 'ngui';
import Mynavpage from './public';
import { Navbar, Toolbar } from 'ngui/nav';
import './review';

function hide_show_navbar(evt) {
  var navbar = evt.sender.topCtr.navbar;
  var hidden = !navbar.hidden
  navbar.setHidden(hidden, true);
  evt.sender.prev.transition({ height: hidden ? 20 : 0, time: 400 });
}

function hide_show_toolbar(evt) {
  var toolbar = evt.sender.topCtr.toolbar;
  toolbar.setHidden(!toolbar.hidden, true);
}

function nav_pop(evt) {
  evt.sender.topCtr.collection.pop(1);
}

function view_code(evt) {
  evt.sender.topCtr.collection.push(review.vx, 1);
}

const navbar_vx = (
  <Navbar backgroundColor="#333" backTextColor="#fff" titleTextColor="#fff">
    <Indep alignX="right" alignY="center" x=-10>
      <Button textFamily="icon" textColor="#fff" textSize=20>\ued63</Button>
    </Indep>
  </Navbar>
)

const toolbar_vx = (
  <Toolbar backgroundColor="#333">
    <Hybrid textAlign="center" width="full" height="full">
      <Button onClick=view_code>
        <Text class="toolbar_btn" textColor="#fff">\ue9ab</Text>
      </Button>
    </Hybrid>
  </Toolbar>
)

export const vx = (
  <Mynavpage 
    title="Nav" source=resolve(__filename) 
    backgroundColor="#333" navbar=navbar_vx toolbar=toolbar_vx>
    <Div width="full">
      <Div width="full" height=0 />
      <Button class="long_btn2" onClick=hide_show_navbar>Hide/Show Navbar</Button>
      <Button class="long_btn2" onClick=hide_show_toolbar>Hide/Show Toolbar</Button>
      <Button class="long_btn2" onClick=nav_pop>Nav pop</Button>
    </Div>
  </Mynavpage>
)