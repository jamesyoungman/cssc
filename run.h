/*
 * run.h
 *
 * By Ross Ridge
 * Public Domain
 *
 * @(#) MySC run.h 1.1 93/11/09 17:17:49
 *
 */

#ifndef __RUN_H__
#define __RUN_H__

#include "list.h"

#ifdef CONFIG_NO_FORK

#if defined(CONFIG_NO_SPAWN) && !defined(CONFIG_DJGPP)

#define STATUS(n) (0)
#define STATUS_MSG(n) 

#else /* defined(CONFIG_NO_SPAWN) && !defined(CONFIG_DJGPP) */

#define STATUS(n) (n)
#define STATUS_MSG(n) "(status = %d)", (n)

#endif /* defined(CONFIG_NO_SPAWN) && !defined(CONFIG_DJGPP) */

#else /* CONFIG_NO_FORK */

#define STATUS(n) ((n) << 8)
#define STATUS_MSG(n) "(status = %d, %d)", (n) >> 8, (n) & 0xff

#endif /* CONFIG_NO_FORK */

int run(const char *prg, list<const char *> const &args);
int run_mr_checker(const char *prg, const char *arg1,
		   list<mystring> mrs);

#endif /* __RUN_H__ */

/* Local variables: */
/* mode: c++ */
/* End: */
