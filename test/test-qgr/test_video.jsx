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

import { Root, GUIApplication } from 'qgr';
import { AudioPlayer, Video } from 'qgr/media';

const src0 = '/sdcard/Download/Shutter.Island.2010.禁闭岛.双语字幕.HR-HDTV.AC3.1024X576.X264-人人影视制作.mkv';
const src1 = '/sdcard/Download/神鬼奇谋.mp4';
const src2 = '/sdcard/Download/十二生肖BD1280中英双字.mkv';
const src3 = '/sdcard/Download/一代宗师720P.mkv';
const src4 = '/mnt/sdcard2/Shutter.Island.2010.禁闭岛.双语字幕.HR-HDTV.AC3.1024X576.X264-人人影视制作.mkv';
const src5 = '/mnt/sdcard2/神鬼奇谋.mp4';
const src5_1 = '/mnt/sdcard2/Avatar.2009.阿凡达.国英音轨.双语字幕.HR-HDTV.AC3.1024X576.X264-人人影视制作.mkv';
const src5_2 = '/mnt/sdcard2/荒野猎人.DVD高清中英双字.mp4'
const src6 = '/sdcard/Download/9300008.ts';
const src7 = '/sdcard/Download/VID_20121203_210243.3gp';
const src8 = '/sdcard/Download/20150509_164506.mp4';
const src9 = '/sdcard/Download/Avatar.2009.阿凡达.国英音轨.双语字幕.HR-HDTV.AC3.1024X576.X264-人人影视制作.mkv';
const src10 = '/sdcard/Download/荒野猎人.DVD高清中英双字.mp4';
const src10_1 = '/Users/louis/Movies/荒野猎人.DVD高清中英双字.mp4';

const src11 = 'http://111.202.98.148/vipts.tc.qq.com/yNuQ4zyT18jbfgZXdZ5S8VMUx8I8HHDvh20LvQN_1xMY6yndk6xEed8pg6dQi0CBnHxt54-rBKOXgM3WwWcmtnkEDstl3d9jlyDDIPHf4OrRaCRhSO_WTQ/x0020s21tyn.320092.ts.m3u8?ver=4&amp;sdtfrom=v3000&amp;platform=10403&amp;appver=5.4.0.17642&amp;projection=dlna';

const src12 = 'http://192.168.1.101/Videos/Shutter.Island.2010.禁闭岛.双语字幕.HR-HDTV.AC3.1024X576.X264-人人影视制作.mkv';
const src13 = 'http://192.168.1.101/Videos/jbd/1/00.m3u8';
const src14 = 'http://192.168.1.101/Videos/神鬼奇谋.mp4';
const src15 = 'http://192.168.1.101/Videos/X战警：金钢狼_X-Men.Origins.Wolverine_2009_BDRip.rmvb';
const src16 = 'http://192.168.1.101/Videos/十二生肖BD1280中英双字.mkv';
const src17 = 'http://192.168.1.101/Videos/Avatar.2009.阿凡达.国英音轨.双语字幕.HR-HDTV.AC3.1024X576.X264-人人影视制作.mkv';
const src18 = 'http://192.168.1.101/Videos/新机械战警BD双语双字[电影天堂www.dy2018.com].mkv';
const src19 = 'http://192.168.1.101/Videos/荒野猎人.DVD高清中英双字.mp4';

const src20 = 'https://r4---sn-ogueln7y.googlevideo.com/videoplayback?id=cb36ef41e2569174&itag=133&source=youtube&requiressl=yes&pl=25&ratebypass=yes&mime=video/mp4&gir=yes&clen=67013320&lmt=1467020892144441&dur=2185.186&signature=61143B3299BC39A0B3E256385B52AB46DF46BF85.7675EF5B7B2B60C2FB4F1D929DFC9523E7CD25A2&upn=C4zcAMSz_RM&key=cms1&ip=45.32.255.19&ipbits=0&expire=1489678473&sparams=clen,dur,expire,gir,id,initcwndbps,ip,ipbits,ipbypass,itag,lmt,mime,mm,mn,ms,mv,pl,ratebypass,requiressl,source&redirect_counter=1&req_id=81ede39892cea3ee&cms_redirect=yes&ipbypass=yes&mm=31&mn=sn-ogueln7y&ms=au&mt=1489656809&mv=m';

const src21 = 'http://182.254.20.104/vipts.tc.qq.com/14x0qPPqKRFQDSwsxbfHRwUe4MaDLGx1tDPg5p2niVp1SZH4lKAec9oFnWKpnqCQSEwexaE8QsA2MYm1ZFyUG0fIJchA_5KaKxFLs9m1_wGqjRnE9ba9BQ/g0021kye2o2.320092.ts.m3u8?ver=4&amp;sdtfrom=v3000&amp;platform=10403&amp;appver=5.4.1.17650&amp;projection=dlna';

// var player = new AudioPlayer(src11_1);

new GUIApplication().start(
	<Root background_color="#000">
	  <Video width="100%" margin="auto" src=src21 autoPlay=true />
	</Root>
);
