#include "mysc.h"

#include <time.h>
#undef HAVE_MKTIME
#ifndef HAVE_MKTIME

#define mktime LIDENT(mktime)

#ifdef CONFIG_DECLARE_TIMEZONE
extern long timezone;
#endif
#ifdef CONFIG_DECLARE_TZSET
extern "C" void CDECL tzset();
#endif

/* This is a "good enough for MySC" implementation of mktime. */

time_t
mktime(struct tm const *tm) {
	const long day_secs = 24L * 60L * 60L;
	int year = tm->tm_year - 70;
	time_t t = (long) year * 365 * day_secs
		   + (long) ((year + 1) / 4) * day_secs;
	const long month_offs[12] = {
		0L,
		31L * day_secs,
		59L * day_secs,
		90L * day_secs,
		120L * day_secs,
		151L * day_secs,
		181L * day_secs,
		212L * day_secs,
		243L * day_secs,
		273L * day_secs,
		304L * day_secs,
		334L * day_secs
	};

	t += month_offs[tm->tm_mon] + (long) (tm->tm_mday - 1) * day_secs;
	if (tm->tm_mon > 2 && tm->tm_year % 4 == 0) {
		t += day_secs;
        }

	t += tm->tm_sec + tm->tm_min * 60 + (long) tm->tm_hour * 60 * 60;

#ifdef HAVE_TIMEZONE
	tzset();
	t += timezone;
#endif

	if (localtime(&t)->tm_isdst > 0) {
		t -= 60 * 60;
	}

	return t;
}	       
	
#endif /* HAVE_MKTIME */   

