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

#include "sys.h"
#include "string.h"
//#include <iostream>

#if XX_UNIX
#include <sys/utsname.h>
#include <unistd.h>
#endif

#if XX_APPLE
#include <mach/mach_time.h>
#include <mach/mach.h>
#include <mach/clock.h>

#define clock_gettime clock_gettime2

static clock_serv_t get_clock_port(clock_id_t clock_id) {
  clock_serv_t clock_r;
  host_get_clock_service(mach_host_self(), clock_id, &clock_r);
  return clock_r;
}

static clock_serv_t clock_realtime = get_clock_port(CALENDAR_CLOCK);
static mach_port_t clock_monotonic = get_clock_port(SYSTEM_CLOCK);

int clock_gettime2(clockid_t id, struct timespec *tspec) {
  mach_timespec_t mts;
  int retval = 0;
  if (id == CLOCK_MONOTONIC) {
    retval = clock_get_time(clock_monotonic, &mts);
    if (retval != 0) {
      return retval;
    }
  } else if (id == CLOCK_REALTIME) {
    retval = clock_get_time(clock_realtime, &mts);
    if (retval != 0) {
      return retval;
    }
  } else {
    /* only CLOCK_MONOTOIC and CLOCK_REALTIME clocks supported */
    return -1;
  }
  tspec->tv_sec = mts.tv_sec;
  tspec->tv_nsec = mts.tv_nsec;
  return 0;
}

#endif

XX_NS(ngui)
XX_NS(sys)

String name() {
#if XX_NACL
  static String _name("Nacl"); return _name;
#elif XX_IOS
  static String _name("iOS"); return _name;
#elif XX_OSX
  static String _name("MacOSX"); return _name;
#elif XX_ANDROID
  static String _name("Android"); return _name;
#elif XX_WIN
  static String _name("Windows"); return _name;
#elif XX_QNX
  static String _name("Qnx"); return _name;
#elif XX_LINUX
  static String _name("Linux"); return _name;
#else
# error no Support
#endif
}

#if XX_UNIX

static String _info;

static void initialize() {
  if (_info.is_empty()) {
    static struct utsname _uts;
    static char _hostname[256];
    gethostname(_hostname, 255);
    uname(&_uts);
    _info = String::format("host: %s\nsys: %s\nmachine: %s\nnodename: %s\nversion: %s\nrelease: %s",
                           _hostname,
                           _uts.sysname,
                           _uts.machine,
                           _uts.nodename, _uts.version, _uts.release);
    //  getlogin(), getuid(), getgid(),
  }
}

String info() {
  initialize(); return _info;
}

#endif

bool is_wifi() {
  return network_status() == 2;
}

bool is_mobile() {
  return network_status() >= 3;
}

int64 time_second() {
  return ::time(nullptr);
}

int64 time() {
  timespec now;
  clock_gettime(CLOCK_REALTIME, &now);
  int64_t r = now.tv_sec * 1000000LL + now.tv_nsec / 1000LL;
  return r;
}

int64 time_monotonic() {
  timespec now;
  clock_gettime(CLOCK_MONOTONIC, &now);
  int64_t r = now.tv_sec * 1000000LL + now.tv_nsec / 1000LL;
  return r;
}

XX_END XX_END
