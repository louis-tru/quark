import { Div, Button } from 'ngui';
import Mynavpage from './public';

export const vx = (
  <Mynavpage title="Zlib" source=$(__filename)>
    <Div width="full">
      <Button class="long_btn">OK</Button>
    </Div>
  </Mynavpage>
)