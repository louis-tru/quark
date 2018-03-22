import { Navpage } from 'ngui/nav';
import { 
  ViewController, Button, CSS,
  Text, TextNode, atomPixel: px, 
  Indep, isViewXml, Panel, Scroll, ngui, Style
} from 'ngui';
import 'ngui/util';

// CSS(<Style>
//   .long_btn {
//     margin: 10;
//     margin_bottom: 0;
//     width: full;
//     height: 36;
//     text_line_height: 36;
//     text_color: #0079ff;
//     border_radius: 8;
//     border: ${px} #0079ff;
//   }
//   .long_btn2 {
//     margin: 10;
//     margin_bottom: 0;
//     width: full;
//     height: 36
//     text_line_height: 36;
//     text_color: #fff;
//     border_radius: 8;
//     border: ${px} #fff;
//   }
// </Style>);

CSS({
  
  '.long_btn': {
    margin: 10,
    margin_bottom: 0,
    width: "full",
    height: 36,
    text_line_height: 36,
    text_color: "#0079ff",
    border_radius: 8,
    border: `${px} #0079ff`,
    // border: `2 #0079ff`,
    // background_color: '#f00',
    // border_radius: 80,
    // border: `40 #0079ff`,
    // border_left_color: '#f00',
    // border_right_color: '#f00',
  },
    
  '.long_btn2': {
    margin: 10,
    margin_bottom: 0,
    width: "full",
    height: 36,
    text_line_height: 36,
    text_color: "#fff",
    border_radius: 8,
    border: `${px} #fff`,
  },
  
  '.next_btn': {
    width: "full",
    text_line_height: 45,
    text_align: "left",
    border_radius: 0,
  },
  
  '.next_btn:normal': {
    background_color: '#fff0', time: 180
  },
  
  '.next_btn:hover': {
    background_color: '#ececec', time: 50
  },
  
  '.next_btn:down': {
    background_color: '#E1E4E4', time: 50
  },

  '.input': {
    margin:10,
    margin_bottom:0,
    width:"full",
    height:30,
    background_color:"#eee",
    border_radius:8,
  },

})

export class Navbutton extends ViewController {
  
  loadView(vx) {
    //util.log('---------------------', px);
    super.loadView(
      <Button
        onClick="handle_click"
        class="next_btn"
        textColor="#0079ff"
        defaultHighlighted=0
        borderBottom=`${px} #c8c7cc`>
        <Text marginLeft=16 marginRight=50>${vx}</Text>
        <Indep x=-10 alignX="right" alignY="center">
          <Text value="\uedbe" textFamily="icon" textColor="#aaa" />
        </Indep>
      </Button>
    );
  }
  
  handle_click(evt) {
    if ( isViewXml(this.next) ) {
      var ctr = this.parent;
      while (ctr) {
        if ( ctr instanceof Mynavpage ) {
          ctr.collection.push(this.next, 1); break;
        }
        ctr = ctr.parent;
      }
    }
    // console.log('nav button click');
  }
}

export class Mynavpage extends Navpage {
  source = resolve(__filename);
  
  loadView(vx) {
    super.loadView(vx);
    this.backgroundColor = "#f8f8f8";
    <!-- White title -->
    <!--
      ngui.displayPort.setStatusBarStyle(1);
      this.navbar.backgroundColor = '#f9f9f9';
      this.navbar.titleTextColor = '#000';
      this.navbar.backTextColor = '#0079ff';
    -->
  }

  triggerForeground(e) {
    super.triggerForeground(e);
    /*
    // test TV keyboard
    var btn = null;
    var first = this.view.first;
    if (first && first instanceof Panel) {
      first.allow_leave = true;
      if ( first instanceof Scroll ) {
        first.enable_switch_scroll = true;
      }
      btn = first.first_button();
    }
    if ( !btn ) {
      btn = ngui.root.first_button();
    }
    if ( btn ) {
      btn.focus();
    }
    */
  }

}
