/**
 * \file aos_terminal.h
 * \brief 
 */

#ifndef INCLUDE_AOS_AOS_TERMINAL_H_
#define INCLUDE_AOS_AOS_TERMINAL_H_


#include <aos/aos.h>

typedef size_t (*aos_terminal_send_func_t)(const char *str, size_t length);
typedef size_t (*aos_terminal_recv_func_t)(const char *str, size_t length);

void aos_terminal_set_send_to_serial(aos_terminal_send_func_t f);
aos_terminal_send_func_t aos_terminal_get_send_to_serial(void);

void aos_terminal_set_recv_from_serial(aos_terminal_recv_func_t f);
aos_terminal_recv_func_t aos_terminal_get_recv_from_serial(void);

size_t aos_terminal_write(const char *data, size_t length);
size_t aos_terminal_read(char *buf, size_t len);

#endif /* INCLUDE_AOS_AOS_TERMINAL_H_ */
