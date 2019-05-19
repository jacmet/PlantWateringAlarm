#ifndef _OREGON_H_
#define _OREGON_H_

void oregon_init(unsigned char id);

void oregon_send(unsigned char battery, int temp, int humidity);

#endif /* _OREGON_H_ */
