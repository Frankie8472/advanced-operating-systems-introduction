#ifndef __AOS_M0_H__
#define __AOS_M0_H__

#include <errors/errno.h>

errval_t aos_serial_init(void);
void aos_serial_putchar(char c);
char aos_serial_getchar(void);

void aos_blink_led(void);

/* Enable solution macro */
#define AOS_SOLUTION_M0

#endif // __AOS_M0_H__
