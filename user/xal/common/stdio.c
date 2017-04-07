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
 * COPYRIGHT (c) Fent Innovative Software Solutions S.L.
 *     Read LICENSE.txt file for the license.terms.
 */

#include <stdarg.h>
#include <string.h>
#include <xm.h>

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

#define ABS(v) ((v < 0) ? -v: v)
xm_u32_t vsprintf(char *s, const char *fmt, va_list ap)
{
	const char *p;
	char *bs, *sval;
	char buf[SCRATCH];
	xm_u32_t n, dofmt, islong, base;
	xm_u64_t uval;
	xm_s64_t ival;

	bs = s;
	for (p = fmt; *p; p++) {
		dofmt = 0;
		if (*p == '%'){
			dofmt = 1;
			islong = 0;
		} else
			*s++ = *p;

		while(dofmt)
		switch (*++p) {
		case 'l': {
			islong++;
			break;
		}
		case 'u':
		case 'd':
		case 'x': {
			base=10;
			if (*p == 'x')
				base=16;
			if (*p == 'd'){
				ival = islong > 1? va_arg(ap, xm_s64_t): va_arg(ap, xm_s32_t);
				n = itoa(ABS(ival), buf, base);
				if (ival < 0) buf[--n] = '-';
			} else {
				uval = islong > 1? va_arg(ap, xm_u64_t): va_arg(ap, xm_u32_t);
				n = itoa(uval, buf, base);
			}
			s = strcpy(s, &buf[n]) + (sizeof(buf)-1 - n);
			dofmt = 0;
			break;
		}
		case 's': {
			sval = va_arg(ap, char *);
			s = strcpy(s, sval) + strlen(sval);
			dofmt = 0;
			break;
		}
		default:
			*s++ = *p;
			break;
		}
	}
	return (s - bs);
}

int vprintf(const char *fmt, va_list ap)
{
	int n;
	static char str[1024];

	n = vsprintf(str, fmt, ap);
	uartputs(str, n);

	return n;
}

int sprintf(char *s, const char *fmt, ...)
{
    va_list ap;
    int n = 0;

	va_start(ap, fmt);
	n = vsprintf(s, fmt, ap);
	va_end(ap);

	return n;
}

int printf(const char *fmt, ...)
{
    va_list ap;
    int n = 0;

	va_start(ap, fmt);
	n = vprintf(fmt, ap);
	va_end(ap);

	return n;
}
