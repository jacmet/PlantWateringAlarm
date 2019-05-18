/* Optimized AVR305 half-duplex serial uart implementation
 * Based on BasicSerial.S by Ralph Doncaster
 * timing for 81N, 115.2kbps @8Mhz = 69.4 cycles/bit
 * and @16Mhz = 138.9 cycles/bit
 * @author: Ralph Doncaster
 * @version: $Id$
 */

#include <avr/io.h>


#define BAUD_RATE 9600

; correct for avr/io.h 0x20 port offset for io instructions
#define UART_PORT (PORTA-0x20)
#define UART_TX 5

#ifndef F_CPU
  #error CPU frequency F_CPU undefined
#endif

; iterations of delay loop to reach baud rate. Overhead of transfer is
 ; 9 cycles/bit, and each iteration of delay loop takes 3 cycles
#define TXDELAY (((F_CPU/BAUD_RATE)-9)/3)

#if TXDELAY > 255
  #error low baud rates unsupported - use higher BAUD_RATE
#endif

.global uinit
uinit:
        sbi UART_PORT-1, UART_TX  	        ; set Tx line to output
        sbi UART_PORT, UART_TX                  ; transmit a 1
	ret

.global uputchar
; void uputchar(char c)
; transmit byte in r24 - 15 instructions
; calling code must set Tx line to idle state (high) or 1st byte may be lost
; i.e. PORT |= (1<<UART_Tx)
uputchar:
	cpi r24, '\n'
	brne txstart
	ldi r24, '\r'				; send \n as \r\n
	rcall uputchar
	ldi r24, '\n'
txstart:
	cli
        sbi UART_PORT-1, UART_TX  	        ; set Tx line to output
        ldi r18, 10                             ; 1 start + 8 bit + 1 stop
        com r24	                                ; invert and set carry
txloop:
        ; 9 cycle loop + delay
        brcc tx1
        cbi UART_PORT, UART_TX                  ; transmit a 0
tx1:
        brcs txdone
        sbi UART_PORT, UART_TX                  ; transmit a 1
txdone:
	ldi r19, TXDELAY
txdelay:
; delay (3 cycle * delayArg) -1
        dec r19
        brne txdelay
        lsr r24
        dec r18
        brne txloop
	reti				        ; return and enable interrupts
