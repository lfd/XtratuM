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

#include <stdc.h>

#ifndef CONFIG_OUTPUT_ENABLED
xm_s32_t xprintf(const char *fmt, ...){return 0;}
#else

static void uartputs(char *s, int n)
{
    int i;

    for(i=0; i<n; i++)
        ConsolePutChar(s[i]);
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
static xm_u32_t vsprintf(char *s, const char *fmt, va_list ap)
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

int xprintf(const char *fmt, ...)
{
    va_list ap;
    int n = 0;

	va_start(ap, fmt);
	n = vprintf(fmt, ap);
	va_end(ap);

	return n;
}

#endif

void *memcpy(void *dest, const void *src, xm_u32_t n) {
    xm_s8_t *dp = dest;
    const xm_s8_t *sp = src;
    while (n--)
        *dp++ = *sp++;
    return dest;
}
