
/**
  * @file      doxygen.h
  * @author    JonesLee
  * @email     louistru@hotmail.com
  * @version   V4.01
  * @date      07-DEC-2017
  * @license   GNU General Public License (GPL)  
  * @brief     Universal Synchronous/Asynchronous Receiver/Transmitter 
  * @details    detail
  * @attention
  *  This file is part of OST.                                                  \n
  *  This program is free software; you can redistribute it and/or modify 		\n
  *  it under the terms of the GNU General Public License version 3 as 		    \n
  *  published by the Free Software Foundation.                               	\n 
  *  You should have received a copy of the GNU General Public License   		\n      
  *  along with OST. If not, see <http://www.gnu.org/licenses/>.       			\n  
  *  Unless required by applicable law or agreed to in writing, software       	\n
  *  distributed under the License is distributed on an "AS IS" BASIS,         	\n
  *  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.  	\n
  *  See the License for the specific language governing permissions and     	\n  
  *  limitations under the License.   											\n
  *   																			\n
  * @htmlonly
  * <span style="font-weight: bold">History</span>
  * @endhtmlonly
  * Version|Auther|Date|Describe
  * ------|----|------|-------- 
  * V3.3|Jones Lee|07-DEC-2017|Create File
  * <h2><center>&copy;COPYRIGHT 2017 WELLCASA All Rights Reserved.</center></h2>
  */

extern "C" {

	typedef int CAN_TypeDef;

	/**
		* @brief		can send the message
		* @param[in]	canx : The Number of CAN
		* @param[in]	id : the can id
		* @param[in]	p : the data will be sent
		* @param[in]	size : the data size
		* @param[in]	is_check_send_time : is need check out the time out
		* @note Notice that the size of the size is smaller than the size of the buffer.
		* @return
		*	+1 Send successfully \n
		*	-1 input parameter error \n
		*	-2 canx initialize error \n
		*	-3 canx time out error \n
		* @par Sample
		* @code
		*	u8 p[8] = {0};
		*	res_ res = 0; 
		* 	res = can_send_msg(CAN1,1,p,0x11,8,1);
		* @endcode
		*/							
	short int can_send_msg(const CAN_TypeDef * canx,
							const unsigned int id,
							const unsigned char *p,
							const unsigned char size,
							const unsigned char is_check_send_time);

	/**
	 * @brief test function demo
	 * @param[in] a test input param
	 * @return in a or zero
	 * @retval int
	 * @code
	 u8 p[8] = {0};
	 *	res_ res = 0;
	 *	res = can_send_msg(CAN1,1,p,0x11,8,1);
	 *	@endcode
	*/
	int test(int a) {
		return a ? a: 0;
	}
}

// ------------------------------------------------------------
// natspec规范并不完全兼容doxygen规范
/*
所有标签都是可选的。下表说明了每个natspec标签的用途和使用位置。作为一种特殊情况，如果不使用标记，那么solidity编译器将解释 /// 或 /** 以同样的方式发表评论 @notice .

@title 描述合同/接口的标题
@author 作者姓名
@notice 向最终用户解释它的作用
@dev 向开发人员解释任何额外的细节
@param 记录参数，就像在DOOXO中一样(后面必须跟参数名称)
@return 记录契约函数的返回变量
@inheritdoc 从基函数复制所有缺少的标记（后面必须跟合同名称）
@custom:...
自定义标记，语义是应用程序定义的
*/