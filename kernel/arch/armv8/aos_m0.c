#include <kernel.h>
#include <offsets.h>

#include <aos_m0.h>


errval_t aos_serial_init(void) {
    /* XXX - You'll need to implement this. */
    return SYS_ERR_OK;
}

void aos_serial_putchar(char c) {
    /* XXX - You'll need to implement this. */
}

__attribute__((noreturn))
char aos_serial_getchar(void) {
    /* XXX - You'll need to implement this
     * if you go for the extra challenge
     */
    panic("Unimplemented\n");
}

__attribute__((noreturn))
void aos_blink_led(void) {
    /* XXX - You'll need to implement this. */
    while (1);
}
