/* ***** BEGIN LICENSE BLOCK *****
 * Distributed under the BSD license:
 *
 * Copyright (c) 2015, blue.chu
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *     * Redistributions of source code must retain the above copyright
 *       notice, this list of conditions and the following disclaimer.
 *     * Redistributions in binary form must reproduce the above copyright
 *       notice, this list of conditions and the following disclaimer in the
 *       documentation and/or other materials provided with the distribution.
 *     * Neither the name of blue.chu nor the
 *       names of its contributors may be used to endorse or promote products
 *       derived from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL blue.chu BE LIABLE FOR ANY
 * DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 * ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 * ***** END LICENSE BLOCK ***** */

/*
	A brief summary of the date string formats this parser groks:

	RFC 2616 3.3.1

	Sun, 06 Nov 1994 08:49:37 GMT  ; RFC 822, updated by RFC 1123
	Sunday, 06-Nov-94 08:49:37 GMT ; RFC 850, obsoleted by RFC 1036
	Sun Nov  6 08:49:37 1994       ; ANSI C's asctime() format

	we support dates without week day name:

	06 Nov 1994 08:49:37 GMT
	06-Nov-94 08:49:37 GMT
	Nov  6 08:49:37 1994

	without the time zone:

	06 Nov 1994 08:49:37
	06-Nov-94 08:49:37

	weird order:

	1994 Nov 6 08:49:37  (GNU date fails)
	GMT 08:49:37 06-Nov-94 Sunday
	94 6 Nov 08:49:37    (GNU date fails)

	time left out:

	1994 Nov 6
	06-Nov-94
	Sun Nov 6 94

	unusual separators:

	1994.Nov.6
	Sun/Nov/6/94/GMT

	commonly used time zone names:

	Sun, 06 Nov 1994 08:49:37 CET
	06 Nov 1994 08:49:37 EST

	time zones specified using RFC822 style:

	Sun, 12 Sep 2004 15:05:58 -0700
	Sat, 11 Sep 2004 21:32:11 +0200

	compact numerical date strings:

	20040912 15:05:58 -0700
	20040911 +0200

*/

#include <stdlib.h>
#include <limits.h>
#include <errno.h>
#include <string>
#include <time.h>
#include "./util.h"

namespace qk {

	#define FALSE 0
	#define TRUE 1
	#define ISALPHA(x)  (isalpha((int)  ((unsigned char)x)))
	#define ISDIGIT(x)  (isdigit((int)  ((unsigned char)x)))
	#define ISALNUM(x)  (isalnum((int)  ((unsigned char)x)))

	#if Qk_WIN
	# define ERRNO         ((int)GetLastError())
	# define SET_ERRNO(x)  (SetLastError((DWORD)(x)))
	#else
	# define ERRNO         (errno)
	# define SET_ERRNO(x)  (errno = (x))
	#endif

	/* The size of `time_t', as computed by sizeof. */
	#if Qk_ARCH_64BIT
	# define SIZEOF_TIME_T 8
	#else
	# define SIZEOF_TIME_T 4
	#endif

	cChar * const wkday[] =
	{"Mon", "Tue", "Wed", "Thu", "Fri", "Sat", "Sun"};
	cChar * const wkday2[] =
	{"Sun", "Mon", "Tue", "Wed", "Thu", "Fri", "Sat"};
	static cChar * const weekday[] =
	{ "Monday", "Tuesday", "Wednesday", "Thursday",
		"Friday", "Saturday", "Sunday" };
	cChar * const month[]=
	{ "Jan", "Feb", "Mar", "Apr", "May", "Jun",
		"Jul", "Aug", "Sep", "Oct", "Nov", "Dec" };

	struct tzinfo {
		Char name[5];
		int offset; /* +/- in minutes */
	};

	/*
	 * parsedate()
	 *
	 * Returns:
	 *
	 * PARSEDATE_OK     - a fine conversion
	 * PARSEDATE_FAIL   - failed to convert
	 * PARSEDATE_LATER  - time overflow at the far end of time_t
	 * PARSEDATE_SOONER - time underflow at the low end of time_t
	 */

	static int parsedate(cChar *date, time_t *output);

	#define PARSEDATE_OK     0
	#define PARSEDATE_FAIL   -1
	#define PARSEDATE_LATER  1
	#define PARSEDATE_SOONER 2

	/* Here's a bunch of frequently used time zone names. These were supported
	 by the old getdate parser. */
	#define tDAYZONE -60       /* offset for daylight savings time */
	static const struct tzinfo tz[]= {
		{"GMT", 0},              /* Greenwich Mean */
		{"UTC", 0},              /* Universal (Coordinated) */
		{"WET", 0},              /* Western European */
		{"BST", 0 tDAYZONE},     /* British Summer */
		{"WAT", 60},             /* West Africa */
		{"AST", 240},            /* Atlantic Standard */
		{"ADT", 240 tDAYZONE},   /* Atlantic Daylight */
		{"EST", 300},            /* Eastern Standard */
		{"EDT", 300 tDAYZONE},   /* Eastern Daylight */
		{"CST", 360},            /* Central Standard */
		{"CDT", 360 tDAYZONE},   /* Central Daylight */
		{"MST", 420},            /* Mountain Standard */
		{"MDT", 420 tDAYZONE},   /* Mountain Daylight */
		{"PST", 480},            /* Pacific Standard */
		{"PDT", 480 tDAYZONE},   /* Pacific Daylight */
		{"YST", 540},            /* Yukon Standard */
		{"YDT", 540 tDAYZONE},   /* Yukon Daylight */
		{"HST", 600},            /* Hawaii Standard */
		{"HDT", 600 tDAYZONE},   /* Hawaii Daylight */
		{"CAT", 600},            /* Central Alaska */
		{"AHST", 600},           /* Alaska-Hawaii Standard */
		{"NT",  660},            /* Nome */
		{"IDLW", 720},           /* International Date Line West */
		{"CET", -60},            /* Central European */
		{"MET", -60},            /* Middle European */
		{"MEWT", -60},           /* Middle European Winter */
		{"MEST", -60 tDAYZONE},  /* Middle European Summer */
		{"CEST", -60 tDAYZONE},  /* Central European Summer */
		{"MESZ", -60 tDAYZONE},  /* Middle European Summer */
		{"FWT", -60},            /* French Winter */
		{"FST", -60 tDAYZONE},   /* French Summer */
		{"EET", -120},           /* Eastern Europe, USSR Zone 1 */
		{"WAST", -420},          /* West Australian Standard */
		{"WADT", -420 tDAYZONE}, /* West Australian Daylight */
		{"CCT", -480},           /* China Coast, USSR Zone 7 */
		{"JST", -540},           /* Japan Standard, USSR Zone 8 */
		{"EAST", -600},          /* Eastern Australian Standard */
		{"EADT", -600 tDAYZONE}, /* Eastern Australian Daylight */
		{"GST", -600},           /* Guam Standard, USSR Zone 9 */
		{"NZT", -720},           /* New Zealand */
		{"NZST", -720},          /* New Zealand Standard */
		{"NZDT", -720 tDAYZONE}, /* New Zealand Daylight */
		{"IDLE", -720},          /* International Date Line East */
		/* Next up: Military timezone names. RFC822 allowed these, but (as noted in
		 RFC 1123) had their signs wrong. Here we use the correct signs to match
		 actual military usage.
		 */
		{"A",  +1 * 60},         /* Alpha */
		{"B",  +2 * 60},         /* Bravo */
		{"C",  +3 * 60},         /* Charlie */
		{"D",  +4 * 60},         /* Delta */
		{"E",  +5 * 60},         /* Echo */
		{"F",  +6 * 60},         /* Foxtrot */
		{"G",  +7 * 60},         /* Golf */
		{"H",  +8 * 60},         /* Hotel */
		{"I",  +9 * 60},         /* India */
		/* "J", Juliet is not used as a timezone, to indicate the observer's local
		 time */
		{"K", +10 * 60},         /* Kilo */
		{"L", +11 * 60},         /* Lima */
		{"M", +12 * 60},         /* Mike */
		{"N",  -1 * 60},         /* November */
		{"O",  -2 * 60},         /* Oscar */
		{"P",  -3 * 60},         /* Papa */
		{"Q",  -4 * 60},         /* Quebec */
		{"R",  -5 * 60},         /* Romeo */
		{"S",  -6 * 60},         /* Sierra */
		{"T",  -7 * 60},         /* Tango */
		{"U",  -8 * 60},         /* Uniform */
		{"V",  -9 * 60},         /* Victor */
		{"W", -10 * 60},         /* Whiskey */
		{"X", -11 * 60},         /* X-ray */
		{"Y", -12 * 60},         /* Yankee */
		{"Z", 0},                /* Zulu, zero meridian, a.k.a. UTC */
	};

	/* Portable, consistent toupper (remember EBCDIC). Do not use toupper() because
	 its behavior is altered by the current locale. */
	static Char raw_toupper(Char in)
	{
		if(in >= 'a' && in <= 'z')
			return (Char)('A' + in - 'a');
		return in;
	}

	/*
	 * Curl_raw_equal() is for doing "raw" case insensitive strings. This is meant
	 * to be locale independent and only compare strings we know are safe for
	 * this.  See https://daniel.haxx.se/blog/2008/10/15/strcasecmp-in-turkish/ for
	 * some further explanation to why this function is necessary.
	 *
	 * The function is capable of comparing a-z case insensitively even for
	 * non-ascii.
	 */

	static int raw_equal(cChar *first, cChar *second)
	{
		while(*first && *second) {
			if(raw_toupper(*first) != raw_toupper(*second))
				/* get out of the loop as soon as they don't match */
				break;
			first++;
			second++;
		}
		/* we do the comparison here (possibly again), just to make sure that if the
		 loop above is skipped because one of the strings reached zero, we must not
		 return this as a successful match */
		return (raw_toupper(*first) == raw_toupper(*second));
	}

	/*
	 ** signed long to signed int
	 */

	int sltosi(long slnum)
	{
		return (int)(slnum & (long) 0x7FFFFFFF);
	}

	/* returns:
	 -1 no day
	 0 monday - 6 sunday
	 */

	static int checkday(cChar *check, size_t len)
	{
		int i;
		cChar * const *what;
		bool found= FALSE;
		if(len > 3)
			what = &weekday[0];
		else
			what = &wkday[0];
		for(i=0; i<7; i++) {
			if(raw_equal(check, what[0])) {
				found=TRUE;
				break;
			}
			what++;
		}
		return found?i:-1;
	}

	static int checkmonth(cChar *check)
	{
		int i;
		cChar * const *what;
		bool found= FALSE;
		
		what = &month[0];
		for(i=0; i<12; i++) {
			if(raw_equal(check, what[0])) {
				found=TRUE;
				break;
			}
			what++;
		}
		return found?i:-1; /* return the offset or -1, no real offset is -1 */
	}

	/* return the time zone offset between GMT and the input one, in number
	 of seconds or -1 if the timezone wasn't found/legal */

	static int checktz(cChar *check)
	{
		unsigned int i;
		const struct tzinfo *what;
		bool found= FALSE;
		
		what = tz;
		for(i=0; i< sizeof(tz)/sizeof(tz[0]); i++) {
			if(raw_equal(check, what->name)) {
				found=TRUE;
				break;
			}
			what++;
		}
		return found?what->offset*60:-1;
	}

	static void skip(cChar **date)
	{
		/* skip everything that aren't letters or digits */
		while(**date && !ISALNUM(**date))
			(*date)++;
	}

	enum assume {
		DATE_MDAY,
		DATE_YEAR,
		DATE_TIME
	};

	/* this is a clone of 'struct m' but with all fields we don't need or use
	 cut out */
	struct my_tm {
		int tm_sec;
		int tm_min;
		int tm_hour;
		int tm_mday;
		int tm_mon;
		int tm_year;
	};

	/* struct m to time since epoch in GMT time zone.
	 * This is similar to the standard mktime function but for GMT only, and
	 * doesn't suffer from the various bugs and portability problems that
	 * some systems' implementations have.
	 */
	static time_t my_timegm(struct my_tm *m)
	{
		static const int month_days_cumulative [12] =
		{ 0, 31, 59, 90, 120, 151, 181, 212, 243, 273, 304, 334 };
		int month, year, leap_days;
		
		if(m->tm_year < 70)
			/* we don't support years before 1970 as they will cause this function
			 to return a negative value */
			return -1;
		
		year = m->tm_year + 1900;
		month = m->tm_mon;
		if(month < 0) {
			year += (11 - month) / 12;
			month = 11 - (11 - month) % 12;
		}
		else if(month >= 12) {
			year -= month / 12;
			month = month % 12;
		}
		
		leap_days = year - (m->tm_mon <= 1);
		leap_days = ((leap_days / 4) - (leap_days / 100) + (leap_days / 400)
								 - (1969 / 4) + (1969 / 100) - (1969 / 400));
		
		return ((((time_t) (year - 1970) * 365
							+ leap_days + month_days_cumulative [month] + m->tm_mday - 1) * 24
						 + m->tm_hour) * 60 + m->tm_min) * 60 + m->tm_sec;
	}

	/*
	 * parsedate()
	 *
	 * Returns:
	 *
	 * PARSEDATE_OK     - a fine conversion
	 * PARSEDATE_FAIL   - failed to convert
	 * PARSEDATE_LATER  - time overflow at the far end of time_t
	 * PARSEDATE_SOONER - time underflow at the low end of time_t
	 */

	static int parsedate(cChar *date, time_t *output)
	{
		time_t t = 0;
		int wdaynum=-1;  /* day of the week number, 0-6 (mon-sun) */
		int monnum=-1;   /* month of the year number, 0-11 */
		int mdaynum=-1; /* day of month, 1 - 31 */
		int hournum=-1;
		int minnum=-1;
		int secnum=-1;
		int yearnum=-1;
		int tzoff=-1;
		struct my_tm m;
		enum assume dignext = DATE_MDAY;
		cChar *indate = date; /* save the original pointer */
		int part = 0; /* max 6 parts */
		
		while(*date && (part < 6)) {
			bool found=FALSE;
			
			skip(&date);
			
			if(ISALPHA(*date)) {
				/* a name coming up */
				Char buf[32]="";
				size_t len;
				if(sscanf(date, "%31[ABCDEFGHIJKLMNOPQRSTUVWXYZ"
									"abcdefghijklmnopqrstuvwxyz]", buf))
					len = strlen(buf);
				else
					len = 0;
				
				if(wdaynum == -1) {
					wdaynum = checkday(buf, len);
					if(wdaynum != -1)
						found = TRUE;
				}
				if(!found && (monnum == -1)) {
					monnum = checkmonth(buf);
					if(monnum != -1)
						found = TRUE;
				}
				
				if(!found && (tzoff == -1)) {
					/* this just must be a time zone string */
					tzoff = checktz(buf);
					if(tzoff != -1)
						found = TRUE;
				}
				
				if(!found)
					return PARSEDATE_FAIL; /* bad string */
				
				date += len;
			}
			else if(ISDIGIT(*date)) {
				/* a digit */
				int val;
				Char *end;
				if((secnum == -1) &&
					 (3 == sscanf(date, "%02d:%02d:%02d", &hournum, &minnum, &secnum))) {
					/* time stamp! */
					date += 8;
				}
				else if((secnum == -1) &&
								(2 == sscanf(date, "%02d:%02d", &hournum, &minnum))) {
					/* time stamp without seconds */
					date += 5;
					secnum = 0;
				}
				else {
					long lval;
					int error;
					int old_errno;
					
					old_errno = ERRNO;
					SET_ERRNO(0);
					lval = strtol(date, &end, 10);
					error = ERRNO;
					if(error != old_errno)
						SET_ERRNO(old_errno);
					
					if(error)
						return PARSEDATE_FAIL;
					
	#if LONG_MAX != INT_MAX
					if((lval > (long)INT_MAX) || (lval < (long)INT_MIN))
						return PARSEDATE_FAIL;
	#endif
					
					val = sltosi(lval);
					
					if((tzoff == -1) &&
						 ((end - date) == 4) &&
						 (val <= 1400) &&
						 (indate< date) &&
						 ((date[-1] == '+' || date[-1] == '-'))) {
						/* four digits and a value less than or equal to 1400 (to take into
						 account all sorts of funny time zone diffs) and it is preceded
						 with a plus or minus. This is a time zone indication.  1400 is
						 picked since +1300 is frequently used and +1400 is mentioned as
						 an edge number in the document "ISO C 200X Proposal: Timezone
						 Functions" at http://david.tribble.com/text/c0xtimezone.html If
						 anyone has a more authoritative source for the exact maximum time
						 zone offsets, please speak up! */
						found = TRUE;
						tzoff = (val/100 * 60 + val%100)*60;
						
						/* the + and - prefix indicates the local time compared to GMT,
						 this we need ther reversed math to get what we want */
						tzoff = date[-1]=='+'?-tzoff:tzoff;
					}
					
					if(((end - date) == 8) &&
						 (yearnum == -1) &&
						 (monnum == -1) &&
						 (mdaynum == -1)) {
						/* 8 digits, no year, month or day yet. This is YYYYMMDD */
						found = TRUE;
						yearnum = val/10000;
						monnum = (val%10000)/100-1; /* month is 0 - 11 */
						mdaynum = val%100;
					}
					
					if(!found && (dignext == DATE_MDAY) && (mdaynum == -1)) {
						if((val > 0) && (val<32)) {
							mdaynum = val;
							found = TRUE;
						}
						dignext = DATE_YEAR;
					}
					
					if(!found && (dignext == DATE_YEAR) && (yearnum == -1)) {
						yearnum = val;
						found = TRUE;
						if(yearnum < 1900) {
							if(yearnum > 70)
								yearnum += 1900;
							else
								yearnum += 2000;
						}
						if(mdaynum == -1)
							dignext = DATE_MDAY;
					}
					
					if(!found)
						return PARSEDATE_FAIL;
					
					date = end;
				}
			}
			
			part++;
		}
		
		if(-1 == secnum)
			secnum = minnum = hournum = 0; /* no time, make it zero */
		
		if((-1 == mdaynum) ||
			 (-1 == monnum) ||
			 (-1 == yearnum))
			/* lacks vital info, fail */
			return PARSEDATE_FAIL;
		
	#if SIZEOF_TIME_T < 5
		/* 32 bit time_t can only hold dates to the beginning of 2038 */
		if(yearnum > 2037) {
			*output = 0x7fffffff;
			return PARSEDATE_LATER;
		}
	#endif
		
		if(yearnum < 1970) {
			*output = 0;
			return PARSEDATE_SOONER;
		}
		
		if((mdaynum > 31) || (monnum > 11) ||
			 (hournum > 23) || (minnum > 59) || (secnum > 60))
			return PARSEDATE_FAIL; /* clearly an illegal date */
		
		m.tm_sec = secnum;
		m.tm_min = minnum;
		m.tm_hour = hournum;
		m.tm_mday = mdaynum;
		m.tm_mon = monnum;
		m.tm_year = yearnum - 1900;
		
		/* my_timegm() returns a time_t. time_t is often 32 bits, even on many
		 architectures that feature 64 bit 'long'.
		 
		 Some systems have 64 bit time_t and deal with years beyond 2038. However,
		 even on some of the systems with 64 bit time_t mktime() returns -1 for
		 dates beyond 03:14:07 UTC, January 19, 2038. (Such as AIX 5100-06)
		 */
		t = my_timegm(&m);
		
		/* time zone adjust (cast t to int to compare to negative one) */
		if(-1 != (int)t) {
			
			/* Add the time zone diff between local time zone and GMT. */
			long delta = (long)(tzoff!=-1?tzoff:0);
			
			if((delta>0) && (t > LONG_MAX - delta)) {
				*output = 0x7fffffff;
				return PARSEDATE_LATER; /* time_t overflow */
			}
			
			t += delta;
		}
		
		*output = t;
		
		return PARSEDATE_OK;
	}

	int64_t parse_time(cString& str)
	{
		time_t parsed = -1;
		int rc = parsedate(str.c_str(), &parsed);
		
		switch(rc) {
			case PARSEDATE_OK:
			case PARSEDATE_LATER:
			case PARSEDATE_SOONER:
				return int64_t(parsed) * 1000000LL;
		}
		/* everything else is fail */
		return -1;
	}

	String gmt_time_string(int64_t second) {
		time_t t = second;
		struct tm m;
		memcpy(&m, gmtime(&t), sizeof(struct tm));

		// "Thu, 30 Mar 2017 06:16:55 GMT"
		/*
		struct m{
			int tm_sec;             //取值[0,59)，非正常情况下可到达61
			int tm_min;             //取值同上
			int tm_hour;            //取值[0,23)
			int tm_mday;            //取值(1,31]
			int tm_mon;             //取值[0,11)
			int tm_year;            //1900年起距今的年数
			int tm_wday;            //取值[0,6]
			int tm_yday;            //取值[0，366)
			int tm_isdst;           //夏令时标志
		};*/
		
		String r = String::format("%s, %d%d %s %d %d%d:%d%d:%d%d GMT"
		 , wkday2[m.tm_wday]
		 , m.tm_mday / 10, m.tm_mday % 10
		 , month[m.tm_mon]
		 , m.tm_year + 1900
		 , m.tm_hour / 10, m.tm_hour % 10
		 , m.tm_min / 10, m.tm_min % 10
		 , m.tm_sec / 10, m.tm_sec % 10
		);
		
		return r;
	}

}
