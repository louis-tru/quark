import { Div, Button, Text, Scroll } from 'ngui';
import Mynavpage from './public';

export const vx = (
  <Mynavpage title="About" source=resolve(__filename)>
    <Scroll width="full" height="full">
      <Text width="full" margin=10>
@@Ngui
===============

Used C/C++/OpenGL/javascript to implement a GUI typesetting display engine and cross platform GUI application development framework
Goal: developing GUI applications on this basis can take into account both the simplicity and speed of developing WEB applications, as well as the performance and experience of Native applications.

使用C/C++/OpenGL/javascript实现的一个GUI排版显示引擎与跨平台GUI应用开发框架
目标：在此基础上开发GUI应用程序可兼顾开发WEB应用程序的简单与速度同时拥有Native应用程序的性能与体验.

Ngui Source 
===============
https://github.com/louis-tru/ngui.git

Support
===============
http://nodegui.org
louistru@hotmail.com


      </Text>
    </Scroll>
  </Mynavpage>
)
