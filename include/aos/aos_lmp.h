/**
 * \file
 * \brief LMP Channel Bindings for AOS
 */

/*
 * Copyright (c) 2012-2016, ETH Zurich.
 * All rights reserved.
 *
 * This file is distributed under the terms in the attached LICENSE file.
 * If you do not find this file, copies can be found by writing to:
 * ETH Zurich D-INFK, Haldeneggsteig 4, CH-8092 Zurich. Attn: Systems Group.
 */

#ifndef _LIB_AOS_AOS_LMP_H
#define _LIB_AOS_AOS_LMP_H

#include <aos/aos.h>
#include <aos/lmp_chan.h>
#include <barrelfish_kpi/lmp.h>

#define LMP_PAYLOAD_WORDS ((LMP_MSG_LENGTH) - 1)
#define LMP_PAYLOAD       ((LMP_PAYLOAD_WORDS) * sizeof(uintptr_t))

#include <aos/aos_msg_def.h>

typedef void (*aos_recv_handler_fn)(struct aos_chan *ac);

/* An LMP channel binding supporting AOS message types.  Not thread safe. */
struct aos_chan {
    struct lmp_chan *lc;
    struct waitset *ws;

    /* The PID of the process on the other end of this channel. */
    uint32_t pid;

    /* There can be only one waiting handler at a time - this is it. */
    aos_recv_handler_fn recv_handler;

    struct thread_mutex tx_mutex;

    /* The state of a partially-completed receive. */
    struct thread_mutex rx_mutex;
    struct aos_msg_raw rx_msg;
    size_t rx_pos;
    struct capref rx_cap;
    bool ongoing_receive;

    /* Opaque closure pointer, for the message handler. */
    void *st;

    /* if set, pointer to a shared frame for bulk transfers */
    void *bulk_frame;
};

/* Message type and length are packed into the first word. */
static inline void aos_chan_msg_set_type_and_length(uintptr_t *hdr,
                                                    enum aos_msgtype mtype,
                                                    uintptr_t len)
{
    assert(mtype < 256);
    assert(sizeof(uintptr_t) == 4);
    assert(hdr);
    *hdr = (((uintptr_t)mtype) << 24) | (len & 0xffffff);
}

static inline enum aos_msgtype aos_chan_msg_extract_type(uintptr_t hdr)
{
    return hdr >> 24;
}

static inline uintptr_t aos_chan_msg_extract_size(uintptr_t hdr)
{
    return hdr & 0xffffff;
}

errval_t aos_chan_init(struct aos_chan *ac, struct lmp_chan *lc,
                       struct waitset *ws);
errval_t aos_chan_set_recv_handler(struct aos_chan *ac,
                                   aos_recv_handler_fn newhandler,
                                   aos_recv_handler_fn *oldhandler);
errval_t aos_chan_change_waitset(struct aos_chan *ac,
                                 struct waitset *ws, struct waitset **old_ws);
errval_t aos_chan_send_blocking(struct aos_chan *ac, void *msgptr,
                                struct capref send_cap);

errval_t aos_marshal_string(char *strbuf, const char *string,
                            size_t *send_length);

/* One-way messages. */
errval_t aos_chan_send_empty(struct aos_chan *ac);
errval_t aos_chan_send_number(struct aos_chan *ac, uintptr_t num);
errval_t aos_chan_send_string(struct aos_chan *ac, const char *string);
errval_t aos_chan_send_lmp_ep(struct aos_chan *ac, struct capref ep);
errval_t aos_chan_send_domain_exit(struct aos_chan *ac, int32_t code);
errval_t aos_chan_send_domain_up(struct aos_chan *ac);
errval_t aos_chan_send_errno(struct aos_chan *ac, errval_t err);

errval_t aos_chan_send_new_chan_request(struct aos_chan *ac,
                                        uintptr_t is_ump, uintptr_t state);
errval_t aos_chan_send_new_chan_reply(struct aos_chan *ac,
                                      struct capref ep, uintptr_t state,
                                      errval_t error);

/* XXX - Should this be here? */
size_t aos_chan_terminal_write(struct aos_chan *ac, const char *data,
                               size_t length);

#endif // _LIB_AOS_AOS_LMP_H
