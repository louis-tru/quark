/* ***** BEGIN LICENSE BLOCK *****
 * Distributed under the BSD license:
 *
 * Copyright (c) 2015, xuewen.chu
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *     * Redistributions of source code must retain the above copyright
 *       notice, this list of conditions and the following disclaimer.
 *     * Redistributions in binary form must reproduce the above copyright
 *       notice, this list of conditions and the following disclaimer in the
 *       documentation and/or other materials provided with the distribution.
 *     * Neither the name of xuewen.chu nor the
 *       names of its contributors may be used to endorse or promote products
 *       derived from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL xuewen.chu BE LIABLE FOR ANY
 * DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 * ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 * ***** END LICENSE BLOCK ***** */

#include <flare/util/util.h>

#if FX_LINUX && !FX_ANDROID

#include <stdio.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <fcntl.h>
#include <errno.h>
#include <linux/input.h>

/*
对于有触摸设备的电脑或者手机，通过cat /proc/bus/input/devices应该就能够看到触摸设备的相关信息。比如

~ # cat /proc/bus/input/devices                                                 
I: Bus=0013 Vendor=0x0012 Product=0x1200 Version=0101                               
N: Name="TouchScreen"                                                    
P: Phys=                                                                        
S: Sysfs=/devices/virtual/input/input0                                          
U: Uniq=                                                                        
H: Handlers=event0                                                              
B: EV=b                                                                         
B: KEY=0                                                                        
B: ABS=1000003 
上面的信息有触摸屏vid,pid,版本等，以及ABS表示触摸屏的绝对坐标掩码，掩码上面表示16进制，所以

0x1000003=0001 0000 0000 0000 0000 0000 0011，其中为1的比特的位置就表示触摸屏会报告这一类型的事件，
前面bit0和bit1以及bit24为1，那么看linux/input.h文件就表示事件code码有ABS_X=0x00, ABS_Y=0x01, 
ABS_PRESSURE=0x18，这三个分别表示触摸屏报告触摸的x坐标，y坐标，以及按下和松开。

但是不同的触摸屏的x,y坐标的范围不一定，所以需要通过input_absinfo结构体去记录得到触摸屏的绝对值信息。
*/

int test_linux_input2() 
{
	fd_set	rds;
	int	ret;
	struct input_event	event;
	struct timeval	time;

	int	fd = open( "/dev/input/event0", O_RDONLY );
	if ( fd < 0 ) {
		FX_ERR( "/dev/input/event0" );
		return(-1);
	}

	while (1) {
		FD_ZERO( &rds );
		FD_SET( fd, &rds );
		/*调用select检查是否能够从/dev/input/event0设备读取数据*/
		ret = select( fd + 1, &rds, NULL, NULL, NULL );
		if ( ret < 0 ) 
		{
			FX_ERR( "select" );
			return(-1);
		}
		/*能够读取到数据*/
		else if ( FD_ISSET(fd, &rds) )
		{ 
			ret	= read( fd, &event, sizeof(struct input_event) );
			time	= event.time;
			F_LOG( "timeS=%d,timeUS=%d,type=%d,code=%d,value=%d\n", 
				time.tv_sec, time.tv_usec, event.type, event.code, event.value );
		}
	}

	/*关闭设备文件句柄*/
	close( fd );

	/*
		触摸一下触摸屏，得到下面的事件报告，timeS和timeUS是发生报告的事件，
		对于PC的话一般是从1970到现在的时间，对于我的开发板的话是系统启动开始到报告的时间。

		type的话是事件类型，为3就是EV_ABS=0x03，为0就是EV_SYN=0x00（用来作为事件的分隔）

		code的话根据事件类型而定，如果是EV_ABS的话，那么code就从ABS_XXX中去找，
		type为EV_ABS，code为0就是ABS_X，code为1就是ABS_Y，code为24就是ABS_PRESSURE，
		这些都可以在linux/input.h里面找到，然后value就是在type和code的前提下的值，
		比如type为EV_ABS，code为ABS_X，那么value就代表触摸点的x轴绝对值。

		timeS=3143,timeUS=415046,type=3,code=0,value=325  触摸x轴值
		timeS=3143,timeUS=415082,type=3,code=1,value=599  触摸y轴值
		timeS=3143,timeUS=415092,type=3,code=24,value=1   按下
		timeS=3143,timeUS=415098,type=0,code=0,value=0    同步
		timeS=3143,timeUS=430129,type=3,code=0,value=323  触摸x轴值
		timeS=3143,timeUS=430169,type=3,code=1,value=592  触摸y轴值
		timeS=3143,timeUS=430183,type=0,code=0,value=0    同步
		timeS=3143,timeUS=445130,type=3,code=24,value=0   松开
	*/

	return(0);
}
 
int test_linux_input__(int argc, char **argv)
{
	struct input_absinfo absI;
	int ret;

	int fd = open("/dev/input/event0", O_RDONLY);
	if (fd < 0) {
		FX_ERR("/dev/input/event0"); 
		return -1;
	}

	//得到X轴的abs信息
	ioctl(fd, EVIOCGABS(ABS_X), &absI);
	F_LOG("x abs lastest value=%d\n", absI.value);
	F_LOG("x abs min=%d\n", absI.minimum);
	F_LOG("x abs max=%d\n", absI.maximum);

	//得到y轴的abs信息
	ioctl(fd,EVIOCGABS(ABS_Y), &absI);
	F_LOG("y abs lastest value=%d\n", absI.value);
	F_LOG("y abs min=%d\n", absI.minimum);
	F_LOG("y abs max=%d\n", absI.maximum);

	//得到按压轴的abs信息
	ioctl(fd,EVIOCGABS(ABS_PRESSURE), &absI);
	F_LOG("pressure abs lastest value=%d\n", absI.value);
	F_LOG("pressure abs min=%d\n", absI.minimum);
	F_LOG("pressure abs max=%d\n", absI.maximum);

	close(fd);

	return test_linux_input2();
}

void test_linux_input(int argc, char **argv){
	test_linux_input__(argc, argv);
}

#else
void test_linux_input(int argc, char **argv) {}
#endif
