/* ***** BEGIN LICENSE BLOCK *****
 * Distributed under the BSD license:
 *
 * Copyright (c) 2015, Louis.chu
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *     * Redistributions of source code must retain the above copyright
 *       notice, this list of conditions and the following disclaimer.
 *     * Redistributions in binary form must reproduce the above copyright
 *       notice, this list of conditions and the following disclaimer in the
 *       documentation and/or other materials provided with the distribution.
 *     * Neither the name of Louis.chu nor the
 *       names of its contributors may be used to endorse or promote products
 *       derived from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL Louis.chu BE LIABLE FOR ANY
 * DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 * ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 * ***** END LICENSE BLOCK ***** */

#include <src/util/util.h>
#include "./test.h"

#if Qk_LINUX && !Qk_ANDROID

#include <stdio.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <fcntl.h>
#include <errno.h>
#include <linux/input.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <time.h>

struct input_event event;

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

int test_linux_input_3(int argc, char **argv)
{
	char          name[64];           /* RATS: Use ok, but could be better */
	char          buf[256] = { 0, };  /* RATS: Use ok */
	unsigned char mask[EV_MAX/8 + 1]; /* RATS: Use ok */
	int           version;
	int           fd = 0;
	int           rc;
	int           i, j;
	char          *tmp;

	#define test_bit(bit) (mask[(bit)/8] & (1 << ((bit)%8)))

	for (i = 0; i < 32; i++) {
		sprintf(name, "/dev/input/event%d", i);
		if ((fd = open(name, O_RDONLY, 0)) >= 0) {
			ioctl(fd, EVIOCGVERSION, &version);
			ioctl(fd, EVIOCGNAME(sizeof(buf)), buf);
			ioctl(fd, EVIOCGBIT(0, sizeof(mask)), mask);
			printf("%s\n", name);
			printf("    evdev version: %d.%d.%d\n",
				   version >> 16, (version >> 8) & 0xff, version & 0xff);
			printf("    name: %s\n", buf);
			printf("    features:");
			for (j = 0; j < EV_MAX; j++) {
				if (test_bit(j)) {
					const char *type = "unknown";
					switch(j) {
					case EV_KEY: type = "keys/buttons"; break;
					case EV_REL: type = "relative";     break;
					case EV_ABS: type = "absolute";     break;
					case EV_MSC: type = "reserved";     break;
					case EV_LED: type = "leds";         break;
					case EV_SND: type = "sound";        break;
					case EV_REP: type = "repeat";       break;
					case EV_FF:  type = "feedback";     break;
					}
					printf(" %s", type);
				}
			}
			printf("\n");
			close(fd);
		}
	}

	if (argc > 1) {
		sprintf(name, "/dev/input/event%d", atoi(argv[1]));
		if ((fd = open(name, O_RDWR, 0)) >= 0) {
			printf("%s: open, fd = %d\n", name, fd);
			for (i = 0; i < LED_MAX; i++) {
				event.time.tv_sec  = time(0);
				event.time.tv_usec = 0;
				event.type         = EV_LED;
				event.code         = i;
				event.value        = 0;
				write(fd, &event, sizeof(event));
			}
			
			while ((rc = read(fd, &event, sizeof(event))) > 0) {
				printf("%-24.24s.%06lu type 0x%04x; code 0x%04x;"
					   " value 0x%08x; ",
					   ctime(&event.time.tv_sec),
					   event.time.tv_usec,
					   event.type, event.code, event.value);
				switch (event.type) {
				case EV_KEY:
					if (event.code > BTN_MISC) {
						printf("Button %d %s",
							   event.code & 0xff,
							   event.value ? "press" : "release");
					} else {
						printf("Key %d (0x%x) %s",
							   event.code & 0xff,
							   event.code & 0xff,
							   event.value ? "press" : "release");
					}
					break;
				case EV_REL:
					switch (event.code) {
					case REL_X:      tmp = "X";       break;
					case REL_Y:      tmp = "Y";       break;
					case REL_HWHEEL: tmp = "HWHEEL";  break;
					case REL_DIAL:   tmp = "DIAL";    break;
					case REL_WHEEL:  tmp = "WHEEL";   break;
					case REL_MISC:   tmp = "MISC";    break;
					default:         tmp = "UNKNOWN"; break;
					}
					printf("Relative %s %d", tmp, event.value);
					break;
				case EV_ABS:
					switch (event.code) {
					case ABS_X:        tmp = "X";        break;
					case ABS_Y:        tmp = "Y";        break;
					case ABS_Z:        tmp = "Z";        break;
					case ABS_RX:       tmp = "RX";       break;
					case ABS_RY:       tmp = "RY";       break;
					case ABS_RZ:       tmp = "RZ";       break;
					case ABS_THROTTLE: tmp = "THROTTLE"; break;
					case ABS_RUDDER:   tmp = "RUDDER";   break;
					case ABS_WHEEL:    tmp = "WHEEL";    break;
					case ABS_GAS:      tmp = "GAS";      break;
					case ABS_BRAKE:    tmp = "BRAKE";    break;
					case ABS_HAT0X:    tmp = "HAT0X";    break;
					case ABS_HAT0Y:    tmp = "HAT0Y";    break;
					case ABS_HAT1X:    tmp = "HAT1X";    break;
					case ABS_HAT1Y:    tmp = "HAT1Y";    break;
					case ABS_HAT2X:    tmp = "HAT2X";    break;
					case ABS_HAT2Y:    tmp = "HAT2Y";    break;
					case ABS_HAT3X:    tmp = "HAT3X";    break;
					case ABS_HAT3Y:    tmp = "HAT3Y";    break;
					case ABS_PRESSURE: tmp = "PRESSURE"; break;
					case ABS_DISTANCE: tmp = "DISTANCE"; break;
					case ABS_TILT_X:   tmp = "TILT_X";   break;
					case ABS_TILT_Y:   tmp = "TILT_Y";   break;
					case ABS_MISC:     tmp = "MISC";     break;
					default:           tmp = "UNKNOWN";  break;
					}
					printf("Absolute %s %d", tmp, event.value);
					break;
				case EV_MSC: printf("Misc"); break;
				case EV_LED: printf("Led");  break;
				case EV_SND: printf("Snd");  break;
				case EV_REP: printf("Rep");  break;
				case EV_FF:  printf("FF");   break;
					break;
				}
				printf("\n");
			}
			printf("rc = %d, (%s)\n", rc, strerror(errno));
			close(fd);
		}
	}

	return 0;
}


Qk_TEST_Func(linux_input_2)
{
	fd_set	rds;
	int	ret;
	struct input_event	event;
	struct timeval	time;

	int	fd = open( "/dev/input/event0", O_RDONLY );
	if ( fd < 0 ) {
		Qk_ELog( "/dev/input/event0" );
		return;
	}

	while (1) {
		FD_ZERO( &rds );
		FD_SET( fd, &rds );
		/*调用select检查是否能够从/dev/input/event0设备读取数据*/
		ret = select( fd + 1, &rds, NULL, NULL, NULL );
		if ( ret < 0 ) 
		{
			Qk_ELog( "select" );
			return;
		}
		/*能够读取到数据*/
		else if ( FD_ISSET(fd, &rds) )
		{ 
			ret	= read( fd, &event, sizeof(struct input_event) );
			time	= event.time;
			Qk_Log( "timeS=%d,timeUS=%d,type=%d,code=%d,value=%d\n", 
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
}

Qk_TEST_Func(linux_input_1)
{
	struct input_absinfo absI;
	int ret;

	int fd = open("/dev/input/event0", O_RDONLY);
	if (fd < 0) {
		Qk_ELog("/dev/input/event0"); 
		return;
	}

	//得到X轴的abs信息
	ioctl(fd, EVIOCGABS(ABS_X), &absI);
	Qk_Log("x abs lastest value=%d\n", absI.value);
	Qk_Log("x abs min=%d\n", absI.minimum);
	Qk_Log("x abs max=%d\n", absI.maximum);

	//得到y轴的abs信息
	ioctl(fd,EVIOCGABS(ABS_Y), &absI);
	Qk_Log("y abs lastest value=%d\n", absI.value);
	Qk_Log("y abs min=%d\n", absI.minimum);
	Qk_Log("y abs max=%d\n", absI.maximum);

	//得到按压轴的abs信息
	ioctl(fd,EVIOCGABS(ABS_PRESSURE), &absI);
	Qk_Log("pressure abs lastest value=%d\n", absI.value);
	Qk_Log("pressure abs min=%d\n", absI.minimum);
	Qk_Log("pressure abs max=%d\n", absI.maximum);

	close(fd);
}

Qk_TEST_Func(linux_input) {
	test_linux_input_1(argc, argv, func, assert);
	test_linux_input_2(argc, argv, func, assert);
}

#else
Qk_TEST_Func(linux_input_2) {}
Qk_TEST_Func(linux_input_1) {}
Qk_TEST_Func(linux_input) {}
#endif
