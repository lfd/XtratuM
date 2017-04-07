/*
 * $FILE: stdio.c
 *
 * Standard buffered input/output
 *
 * $VERSION$
 *
 * Author: Salva Peiró <speiro@ai2.upv.es>
 *
 * $LICENSE:
 * (c) Universidad Politecnica de Valencia. All rights reserved.
 *     Read LICENSE.txt file for the license.terms.
 */
/*
 * - Sept 2012 speiro: Minimal printf() initial implementation
 * - Dec 2012  speiro: simplify vsprintf() with fmtflags
 * - Dec 2012  speiro: added vsnprintf() with strings size checks
 */

#include <stdarg.h>
#include <string.h>
#include <xm.h>

#define ABS(v) ((v < 0) ? -v: v)

int putchar(int c) {
    static char buff[512];
    static int i=0;
    if (i > (sizeof(buff)-2)) {
        XM_write_console(&buff[0], i);
        i=0;
    }
    buff[i++]=c;
  
    if (c == '\n' || c == '\r') {
        if (i)
            XM_write_console(&buff[0], i);
        i=0;
    }
  
    return c;
}

static void uartputs(char *s, int n)
{
    int i;

    for(i=0; i<n; i++)
        putchar(s[i]);
}

#define SCRATCH 32
static xm_s32_t itoa(xm_u64_t uval, char a[SCRATCH], xm_u32_t base)
{
	xm_u32_t i, n=SCRATCH-1;
	char hex[16] = "0123456789abcdef";

	if(base < 0 || base > sizeof(hex))
		return n;
	a[n] = '\0';
	do {
		i = uval % base;
		uval /= base;
		a[--n] = hex[i];
	} while(uval);
	return n;
}

#define FLAG_DOFMT 		(1<<0)
#define FLAG_ISLONG		(1<<1)
#define FLAG_ISVLONG	(1<<2)

#define CHECKSTR(strStart, strPtr, strMaxLen) ((strPtr - strStart) >= strMaxLen)

xm_s32_t vsnprintf(char *s, int nc, const char *fmt, va_list ap)
{
	const char *p;
	char *bs, *sval;
	char buf[SCRATCH];
	xm_u32_t n, fmtflag, base;
	xm_u64_t uval;
	xm_s64_t ival;

	bs = s;
	for (p = fmt; *p; p++) {
		if (CHECKSTR(bs, s, nc))
			break;

		fmtflag = 0;
		if (*p == '%'){
			fmtflag |= FLAG_DOFMT;
		} else
			*s++ = *p;

		while(fmtflag & FLAG_DOFMT)
		switch (*++p) {
		case 'l': {
			fmtflag |= (fmtflag & FLAG_ISLONG)? FLAG_ISVLONG: FLAG_ISLONG;
			break;
		}
		case 'u':
		case 'd':
		case 'x': {
			base=10;
			if (*p == 'x')
				base=16;
			if (*p == 'd'){
				ival = (fmtflag & FLAG_ISVLONG)? va_arg(ap, xm_s64_t): va_arg(ap, xm_s32_t);
				n = itoa(ABS(ival), buf, base);
				if (ival < 0) buf[--n] = '-';
			} else {
				uval = (fmtflag & FLAG_ISVLONG)? va_arg(ap, xm_u64_t): va_arg(ap, xm_u32_t);
				n = itoa(uval, buf, base);
			}
			if (CHECKSTR(bs, s+strlen(&buf[n]), nc))
				break;
			s = strcpy(s, &buf[n]) + (sizeof(buf)-1 - n);
			fmtflag = 0;
			break;
				  }
		case 's': {
			sval = va_arg(ap, char *);
			if (CHECKSTR(bs, s+strlen(sval), nc))
				break;
			s = strcpy(s, sval) + strlen(sval);
			fmtflag = 0;
			break;
		}
		default:
			*s++ = *p;
			break;
		}
	}

	return (s - bs);
}

xm_s32_t vsprintf(char *s, const char *fmt, va_list ap)
{
	xm_s32_t n = 0;

	n = vsnprintf(s, 1024, fmt, ap);

	return n;
}

xm_s32_t vprintf(const char *fmt, va_list ap)
{
	xm_s32_t n = 0;
	static char str[1024];

	n = vsnprintf(str, sizeof(str), fmt, ap);
	uartputs(str, n);

	return n;
}

xm_s32_t snprintf(char *s, xm_s32_t nc, const char *fmt, ...)
{
	xm_s32_t n = 0;
	va_list ap;

	memset(s, 0, nc);
	va_start(ap, fmt);
	n = vsnprintf(s, nc, fmt, ap);
	va_end(ap);

	return n;
}

xm_s32_t sprintf(char *str, const char *fmt, ...)
{
	xm_s32_t n = 0;
	va_list ap;

	va_start(ap, fmt);
	n = vsprintf(str, fmt, ap);
	va_end(ap);

	return n;
}

xm_s32_t printf(const char *fmt, ...) 
{
	va_list ap;
	xm_s32_t n = 0;

	va_start(ap, fmt);
	n = vprintf(fmt, ap);
	va_end(ap);

	return n;
}
