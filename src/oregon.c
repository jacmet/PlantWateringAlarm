#include <avr/io.h>
#include <util/delay.h>

/* THGR2228N temperature/humidity sensor */
#define DEVTYPE1	0x1a
#define DEVTYPE2	0x2d

#define CHANNEL		4

/* bit time in micro seconds */
#define BIT_TIME	512

/* pin to use for transfer */
#define TX_PIN		PA4 /* sck */
#define TX_PORT		PORTA
#define TX_DDR		DDRA

/* pin used for module power */
#define PWR_PIN		PA6 /* mosi */
#define PWR_PORT	PORTA
#define PWR_DDR		DDRA


static unsigned char oregon_id;

static inline void SEND_LOW(void) { TX_PORT &= ~_BV(TX_PIN); }
static inline void SEND_HIGH(void) { TX_PORT |= _BV(TX_PIN); }
static inline void PWR_LOW(void) { PWR_PORT &= ~_BV(PWR_PIN); }
static inline void PWR_HIGH(void) { PWR_PORT |= _BV(PWR_PIN); }


static inline void send_one(void)
{
   SEND_LOW();
	_delay_us(BIT_TIME);
   SEND_HIGH();
	_delay_us(BIT_TIME * 2);
   SEND_LOW();
   _delay_us(BIT_TIME);
}

static inline void send_zero(void)
{
  SEND_HIGH();
  _delay_us(BIT_TIME);
  SEND_LOW();
  _delay_us(BIT_TIME * 2);
  SEND_HIGH();
  _delay_us(BIT_TIME);
}

static void send_byte(unsigned char c)
{
	unsigned char i;

	for (i = 0; i < 8; i++) {
		if (c & (1u << i))
			send_one();
		else
			send_zero();
	}
}

/* based on https://pubweb.eng.utah.edu/~nmcdonal/Tutorials/BCDTutorial/BCDConversion.html */
static unsigned int bin2bcd(unsigned char bin)
{
	unsigned int shift, res;
	unsigned char bit, i;

	res = bin;

	for (bit = 0; bit < 8; bit++) {
		for (shift = 8; shift < 20; shift += 4) {
			if (((res >> shift) & 0xf) >= 5) {
				res += 3 << shift;
			}
		}

		res <<= 1;
	}

	return res >> 8;
}

void oregon_init(unsigned char id)
{
	oregon_id = id;

	TX_DDR |= _BV(TX_PIN);
	PWR_DDR |= _BV(PWR_PIN);

	SEND_LOW();
	PWR_LOW();
}

void oregon_send(unsigned char battery, int temp, int humidity)
{
	unsigned char buf[8], i, sum = 0;

	PWR_HIGH();
	_delay_ms(1);
	
	/* device type */
	buf[0] = DEVTYPE1;
	buf[1] = DEVTYPE2;

	/* channel */
	buf[2] = CHANNEL << 4;

	/* id */
	buf[3] = oregon_id;

	/* battery */
	buf[4] = battery ? 0x1 : 0xc;

	/* temperature */
	if (temp < 0) {
		buf[6] = 0x08;
		temp = -temp;
	} else {
		buf[6] = 0;
	}

	buf[5] = bin2bcd(temp);

	/* humidity */
	humidity = bin2bcd(humidity);

	buf[6] |= (humidity << 4);
	buf[7] = humidity >> 4;

	/* checksum */
	for (i = 0; i < sizeof(buf); i++) {
		sum += buf[i] & 0xf;
		sum += buf[i] >> 4;
	}

	/* first nibble not part of checksum */
	sum -= buf[0] & 0xf;


	/* send 2x */
	for (i = 0; i < 2; i++) {
		/* preamble */
		send_byte(0xff);
		send_byte(0xff);

		for (i = 0; i < sizeof(buf); i++)
			send_byte(buf[i]);

		/* checksum */
		send_byte(sum);

		/* postamble */
		send_byte(0x00);

		SEND_LOW();

		if (!i)
			_delay_us(BIT_TIME * 16);
	}

	PWR_LOW();
}
