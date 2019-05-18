#ifndef _UART_H_
#define _UART_H_

#include <avr/pgmspace.h>

void uinit(void);

void uputchar(char data);

void uprintf_p(const PGM_P fmt, ...);

/* save RAM by storing format string in flash */
#define uprintf(fmt, ...) do { \
		static const char __fmt__[] PROGMEM = fmt; \
		uprintf_p(__fmt__, ##__VA_ARGS__); } while (0)

#endif /* _UART_H_ */
