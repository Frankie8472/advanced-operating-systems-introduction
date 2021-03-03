#include <kernel.h>
#include <offsets.h>

#include <aos_m0.h>

#define STAT (*((volatile unsigned long*)(long long) local_phys_to_mem(0x5A090014)))
#define DATA (*((volatile unsigned char*)(long long) local_phys_to_mem(0x5A09001C)))
#define TDRE ((STAT >> 23) & 1U)
#define RDRF ((STAT >> 21) & 1U)
#define DR (*((unsigned long*)(long long) local_phys_to_mem(0x5D0B0000)))
#define GDIR (*((unsigned long*)(long long) local_phys_to_mem(0x5D0B0004)))

errval_t aos_serial_init(void) {
    /* XXX - You'll need to implement this. */
    /* Don't know the usage of this... happy if you can explain/give an example */

    return SYS_ERR_OK;
}

void aos_serial_putchar(char c) {
    /* XXX - You'll need to implement this. */
    /* OK */

    while (TDRE == 0);
    DATA = c;
}

__attribute__((noreturn))
char aos_serial_getchar(void) {
    /* XXX - You'll need to implement this
     * if you go for the extra challenge
     */

    /* Sure, why not. Seems fun.
     *
     * - Ininite looping and checking RDRF/DATA for input.
     * - If input equals character 'b', output some text and blink several times.
     */

    while(1) {
        //for(volatile long i = 0; i<1000000;i++);    // Some delay for stability, if needed
        if (RDRF == 1){

            char data = DATA;    // Each access will remove a character, so only access ONCE per loop!
            printf("%c", data);  // Output every input. Actually the same as "aos_serial_putchar(data);"

            if (data == 'b'){
                printf("\nLet me try to blink for you...");

                int bctr = 0;
                DR |= 1UL << 23;
                while (bctr < 100){
                    GDIR ^= 1UL << 23;
                    for(volatile long i = 0; i<10000000;i++);
                    bctr++;
                }

                printf("\nYes, I did it!\n");
            }
        }
    }

    //panic("Unimplemented\n");
}

__attribute__((noreturn))
void aos_blink_led(void) {
    /* XXX - You'll need to implement this. */
    /* OK */

    DR |= 1UL << 23;
    while (1){
        GDIR ^= 1UL << 23;
        for(volatile long i = 0; i<50000000;i++);
    }
}
