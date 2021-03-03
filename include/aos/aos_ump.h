#ifndef __AOS_UMP_H
#define __AOS_UMP_H

#include <stdint.h>

/* A cache line is 32B on the Cortex A9. */
#define URPC_BYTES 31
#define SPAWN_NAME_MAX ((URPC_BYTES) - 2)

/* The top three bits of the flags byte are the 'channel full', 'fragmented'
 * and 'last fragment' flags.  The low 5 bits are the size of the delivered
 * message, excluding the flags byte (i.e. 0 to 31 bytes). */
#define URPC_F_FULL (1 << 7)
#define URPC_F_FRAG (1 << 6)
#define URPC_F_LAST (1 << 5)

#define URPC_FULL(flags) ((flags) & URPC_F_FULL)
#define URPC_FRAG(flags) ((flags) & URPC_F_FRAG)
#define URPC_LAST(flags) ((flags) & URPC_F_LAST)
#define URPC_LENGTH(flags) ((flags) & 0x1f)

struct urpc_channel {
    uint8_t packet[URPC_BYTES];
    uint8_t flags;
};

struct urpc_pair {
    struct urpc_channel *fore, *back;
};

size_t urpc_receive(struct urpc_channel *chan, uint8_t *out, size_t buflen);
void urpc_send(struct urpc_channel *chan, uint8_t *in, size_t bytes);

static inline size_t
urpc_wait(struct urpc_pair *urpc_pair, uint8_t *msg_out, size_t buflen) {
    return urpc_receive(urpc_pair->back, msg_out, buflen);
}

static inline void
urpc_reply(struct urpc_pair *urpc_pair, uint8_t *msg_in, size_t bytes) {
    urpc_send(urpc_pair->fore, msg_in, bytes);
}

#endif /* __AOS_UMP_H */
