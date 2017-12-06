import 'ngui/util';
import 'ngui/sys';
import {
  GUIApplication, Root, Scroll, CSS, atomPixel: px,
  Div, Hybrid, Clip, Text, Button, TextNode: T, ngui
} from 'ngui';
import { NavpageCollection, Toolbar } from 'ngui/nav';
import { Navbutton, Mynavpage } from './public';
import './examples';
import './about';
import './review';

CSS({
  
  '.category_title': {
    width: 'full',
    text_line_height: 30,
    text_color: '#6d6d72',
    text_size: 14,
    margin: 16,
  },

  '.rm_margin_top': {
    margin_top: 0,
  },

  '.text_mark': {

  },
  
  '.hello': {
    width: 'full',
    text_size:46, 
    text_align:"center",
    text_color:"#000",
    margin: 16,
    margin_top: 18,
    margin_bottom: 18,
  },
  
  '.category': {
    width: 'full',
    border_top: `${px} #c8c7cc`,
    border_bottom: `${px} #c8c7cc`,
    background_color: '#fff',
  },

  '.toolbar_btn': {
    margin: 8,
    text_family: 'icon',
    text_size: 24,
  },

  '.codepre': {
    width:'full',
    margin:10,
    text_color:"#000",
  },

  '.codepre .tag_name': { text_color: '#005cc5' },
  '.codepre .keywork': { text_color: '#d73a49' },
  '.codepre .identifier': { text_color: '#6f42c1' },
  '.codepre .str': { text_color: '#007526' },
  
})

function review_code(evt) {
  evt.sender.topCtr.collection.push(review.vx, 1);
}

const ngui_tools = 'https://github.com/louis-tru/ngui.git';
const ngui_tools_issues_url = 'https://github.com/louis-tru/ngui/issues';
const examples_source = 'https://github.com/louis-tru/ngui.git';
const documents = 'http://nodegui.org/';

function handle_go_to(evt) {
  var url = evt.sender.url;
  if ( url ) {
    ngui.app.openUrl(url);
  }
}

function handle_bug_feedback() {
  ngui.app.sendEmail('louistru@hotmail.com', 'bug feedback');
}

var default_toolbar_vx = (
  <Toolbar>
    <Hybrid textAlign="center" width="full" height="full">
      <Button onClick=review_code>
        <Text class="toolbar_btn">\ue9ab</Text>
      </Button>
    </Hybrid>
  </Toolbar>
)

var ngui_tools_vx = (
  <Mynavpage title="Ngui Tools" source=resolve(__filename)>
    <Div width="full">
      <Hybrid class="category_title">
@@1. You can use nodejs <T textBackgroundColor="#ddd">npm install -g ngui</T>.
2. Or get the node modules from Github.@@
      </Hybrid>
      <Button class="long_btn rm_margin_top" onClick=handle_go_to url=ngui_tools>Go Github</Button>
    </Div>
  </Mynavpage>
)

const examples_source_vx = (
  <Mynavpage title="Examples Source" source=resolve(__filename)>
    <Div width="full">
      <Text class="category_title">You can get the full examples source code from Github.</Text>
      <Button class="long_btn rm_margin_top" onClick=handle_go_to url=examples_source>Go Github</Button>
    </Div>
  </Mynavpage>
)

var documents_vx = (
  <Mynavpage title="Documents" source=resolve(__filename)>
    <Div width="full">
      <Hybrid class="category_title">Now go to <T textColor="#0079ff">nodegui.org</T> to view the document?</Hybrid>
      <Button class="long_btn rm_margin_top" onClick=handle_go_to url=documents>Go Documents</Button>
    </Div>
  </Mynavpage>
)

const bug_feedback_vx = (
  <Mynavpage title="Bug Feedback" source=resolve(__filename)>
    <Div width="full">
      <Hybrid class="category_title">Now go to Github issues list?</Hybrid>
      <Button class="long_btn rm_margin_top" onClick=handle_go_to url=ngui_tools_issues_url>Go Github Issues</Button>
      <Hybrid class="category_title">Or you can send me email, too.</Hybrid>
      <Button class="long_btn rm_margin_top" onClick=handle_bug_feedback>Send email</Button>
    </Div>
  </Mynavpage>
)

var app = new GUIApplication({ multisample: 2, mipmap: 1 }).start(
  <Root>

    <NavpageCollection id="npc" defaultToolbar=default_toolbar_vx>
      <Mynavpage title="Ngui" source=resolve(__filename)>

        <Scroll width="full" height="full" bounceLock=0>
          
          <Text class="hello">Hello.</Text>
          <Div class="category">
            <Hybrid class="codepre">
@@<T class="keywork">import</T> { <T class="identifier">GUIApplication</T>, <T class="identifier">Root</T> } <T class="keywork">from</T> <T class="str">'ngui'</T>
<T class="keywork">new</T> <T class="identifier">GUIApplication</T>()<T class="keywork">.</T><T class="identifier">start</T>(
  \<<T class="tag_name">Root</T>\>hello world!\</<T class="tag_name">Root</T>\>
)@@
            </Hybrid>
          </Div>

          <Text class="category_title" />
          <Clip class="category">
            <Navbutton next=examples.vx>Examples</Navbutton>
            <Navbutton next=examples_source_vx>Examples Source</Navbutton>
            <Navbutton next=ngui_tools_vx view.borderWidth=0>Ngui Tools</Navbutton>
          </Clip>
          
          <Text class="category_title" />
          <Clip class="category">
            <Navbutton next=about.vx>About</Navbutton>
            <Navbutton next=documents_vx>Documents</Navbutton>
            <Navbutton next=bug_feedback_vx view.borderWidth=0>Bug Feedback</Navbutton>
          </Clip>

          <Div height=32 width="full" />
        </Scroll>

      </Mynavpage>
    </NavpageCollection>
  </Root>
)
