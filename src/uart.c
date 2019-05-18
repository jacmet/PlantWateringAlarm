#include <avr/pgmspace.h>
#include <stdarg.h>
#include <ctype.h>
#include "uart.h"

#define SUPPORT_HEX // adds 52 bytes of code
#define SUPPORT_NEGATIVE // adds 22 bytes of code
void uprintf_p(const PGM_P fmt, ...)
{
	char buf[sizeof(int)*3];
	char c, *p;
	unsigned int uval;
	unsigned char div;
	int val;

	va_list ap;

	va_start(ap, fmt);

	while ((c = pgm_read_byte(fmt++)) != '\0') {
		if (c == '%') {
			c = pgm_read_byte(fmt++);
			c = tolower(c);
			// Fetch value [numeric descriptors only]
			switch (c) {
			case 'p':
#ifdef SUPPORT_HEX
				uputchar('0');
				uputchar('x');
#endif
			case 'x':
			case 'd':
			case 'u':
#ifdef SUPPORT_HEX
				if ((c == 'p') || (c == 'x'))
					div = 16;
				else
#endif
					div = 10;

				val = va_arg(ap, int);
#ifdef SUPPORT_NEGATIVE
				if (c == 'd') {
					if (val < 0) {
						uputchar('-');
						val = -val;
					}
				}
#endif
				uval = val;
				p = buf;
				do {
					*p = '0' + uval % div;
#ifdef SUPPORT_HEX
					if (*p > '9') /* compensate for hex digits */
						*p += 'a' - '9' - 1;
#endif
					p++;
					uval /= div;
				} while (uval);

				do {
					uputchar(*--p);
				} while (p != buf);

				break;

			case 's':
				p = va_arg(ap, char *);
				while (*p)
					uputchar(*p++);
				break;

			case 'c':
				c = va_arg(ap, int /*char*/);
				uputchar(c);
				break;

			default:
				uputchar('%');
				if (c != '%')
					uputchar(c);
				break;
			}
	} else
			uputchar(c);
	}
	va_end(ap);
}
