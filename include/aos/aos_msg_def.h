/**
 * \file
 * \brief Message definitions for the AOS message passing channels
 */

/*
 * Copyright (c)  2016 ETH Zurich.
 * All rights reserved.
 *
 * This file is distributed under the terms in the attached LICENSE file.
 * If you do not find this file, copies can be found by writing to:
 * ETH Zurich D-INFK, Universitaetsstrasse 6, CH-8092 Zurich. Attn: Systems Group.
 */

#ifndef _LIB_AOS_AOS_MSG_DEF_H
#define _LIB_AOS_AOS_MSG_DEF_H

#include <aos/aos_lmp.h> /* for LMP_PAYLOAD */

enum aos_msgtype {
    /* One-way LMP messages. */
    AOS_MSGTYPE_NONE,
    AOS_MSGTYPE_EMPTY,
    AOS_MSGTYPE_NUMBER,
    AOS_MSGTYPE_STR,
    AOS_MSGTYPE_EP,
    AOS_MSGTYPE_ERRNO,
    AOS_MSGTYPE_DOMAIN_UP,
    AOS_MSGTYPE_DOMAIN_EXIT,

    /* RPCs. */

    AOS_MSGTYPE_RAMALLOC,
    AOS_MSGTYPE_RAMALLOC_REPLY,

    AOS_MSGTYPE_RAMFREE,
    AOS_MSGTYPE_RAMFREE_REPLY,

    AOS_MSGTYPE_NEWDOM,
    AOS_MSGTYPE_NEWDOM_REPLY,

    AOS_MSGTYPE_REGISTER_IRQ,

    AOS_MSGTYPE_TERM_WRITE,
    /* AOS_MSGTYPE_ERRNO */

    AOS_MSGTYPE_TERM_READ,
    AOS_MSGTYPE_TERM_READ_REPLY,

    AOS_MSGTYPE_DOMAIN_NAME,
    AOS_MSGTYPE_DOMAIN_NAME_REPLY,

    AOS_MSGTYPE_DOMAIN_PIDS,
    AOS_MSGTYPE_DOMAIN_PIDS_REPLY,

    /* These messages are only valid between privileged processes. */

    /* Transfer unused RAM to core 1. */
    AOS_MSGTYPE_RAM_TRANSFER,
    AOS_MSGTYPE_RAM_TRANSFER_REPLY,

    /* Request the bootinfo frame. */
    AOS_MSGTYPE_GET_BOOTINFO,
    AOS_MSGTYPE_GET_BOOTINFO_REPLY,

    /* Spawn a boot image process. */
    AOS_MSGTYPE_SPAWN_BOOTINFO,
    AOS_MSGTYPE_SPAWN_BOOTINFO_REPLY,

    /* Obtain device capabilities. */
    AOS_MSGTYPE_GET_DEVCAP,
    AOS_MSGTYPE_GET_DEVCAP_REPLY,

    /* Wait for the given domain to exit. */
    AOS_MSGTYPE_DOMAIN_WAIT,
    AOS_MSGTYPE_DOMAIN_WAIT_REPLY,

    /* nameservices services */
    AOS_MSGTYPE_NS_REGISTER,
    /* AOS_MSGTYPE_ERRNO */

    AOS_MSGTYPE_NS_LOOKUP,
    AOS_MSGTYPE_NS_LOOKUP_REPLY,

    AOS_MSGTYPE_NEW_CHAN,
    AOS_MSGTYPE_NEW_CHAN_REPLY,

    /* for block service */
    AOS_MSGTYPE_BULK_SETUP,
    /* AOS_MSGTYPE_ERRNO */

    AOS_MSGTYPE_BULK_READ,
    AOS_MSGTYPE_BULK_READ_REPLY,

    AOS_MSGTYPE_BULK_WRITE,
    AOS_MSGTYPE_BULK_WRITE_REPLY,

    /* For distributed caps */
    AOS_MSGTYPE_DISTOPS_DELETE,
    AOS_MSGTYPE_DISTOPS_DELETE_REPLY,
    AOS_MSGTYPE_DISTOPS_RETYPE,
    AOS_MSGTYPE_DISTOPS_RETYPE_REPLY,
    AOS_MSGTYPE_DISTOPS_REVOKE,
    AOS_MSGTYPE_DISTOPS_REVOKE_REPLY,

    /* Inter-init messages for revoke */
    AOS_MSGTYPE_INTERINIT_REVOKE_MARK,
    AOS_MSGTYPE_INTERINIT_REVOKE_MARK_REPLY,
    AOS_MSGTYPE_INTERINIT_REVOKE_COMMIT,
    AOS_MSGTYPE_INTERINIT_REVOKE_COMMIT_REPLY,

    /* Inter-init messages for cap transfer,ownership management */
    AOS_MSGTYPE_INTERINIT_CAP_TRANSFER,
    AOS_MSGTYPE_INTERINIT_CAP_TRANSFER_REPLY,
    AOS_MSGTYPE_INTERINIT_MOVE_OWNERSHIP,
    AOS_MSGTYPE_INTERINIT_MOVE_OWNERSHIP_REPLY,

    /* Inter-init messages for retype */
    AOS_MSGTYPE_INTERINIT_RETYPE_CHECK,
    AOS_MSGTYPE_INTERINIT_RETYPE_CHECK_REPLY,
    AOS_MSGTYPE_INTERINIT_RETYPE_FORWARD,
    AOS_MSGTYPE_INTERINIT_RETYPE_FORWARD_REPLY,

    /* Inter-init messages for delete */
    AOS_MSGTYPE_INTERINIT_DELETE_FOREIGNS,
    AOS_MSGTYPE_INTERINIT_DELETE_FOREIGNS_REPLY,

    /* RAMFS service */
    AOS_MSGTYPE_RAMFS_MKDIR,
    AOS_MSGTYPE_RAMFS_MKDIR_REPLY,
    AOS_MSGTYPE_RAMFS_INIT,
    AOS_MSGTYPE_RAMFS_INIT_REPLY,
    AOS_MSGTYPE_RAMFS_RMDIR,
    AOS_MSGTYPE_RAMFS_RMDIR_REPLY,
    AOS_MSGTYPE_RAMFS_NEW_FILE,
    AOS_MSGTYPE_RAMFS_NEW_FILE_REPLY,
    AOS_MSGTYPE_RAMFS_RM,
    AOS_MSGTYPE_RAMFS_RM_REPLY,
    AOS_MSGTYPE_RAMFS_GETDIR,
    AOS_MSGTYPE_RAMFS_GETDIR_REPLY,
    AOS_MSGTYPE_RAMFS_WRITE,
    AOS_MSGTYPE_RAMFS_WRTTE_REPLY,
    AOS_MSGTYPE_RAMFS_READ,
    AOS_MSGTYPE_RAMFS_READ_REPLY,

    /* SDMA service */
    AOS_MSGTYPE_SDMA_SET_DEST,
    AOS_MSGTYPE_SDMA_COPY,
    AOS_MSGTYPE_SDMA_FILL,

    /* This has to be last. */
    AOS_MSGTYPE_MAX
};

/* XXX - what's this checking, and why? */
STATIC_ASSERT(AOS_MSGTYPE_MAX < (1 << (sizeof(uint8_t)*8)),
              "Message type must be less than 128.");

/* All messages are sized to be one 32-bit word (the header), plus zero or
 * more payload blocks (the body), sized to match the LMP payload (the header
 * is resent on every LMP message). */

#define AOS_MSG_HDR (sizeof(uintptr_t))

#define AOS_MSG_BLOCK LMP_PAYLOAD

#define AOS_MSG_BODY 512

// TODO CHECK THIS! 
//STATIC_ASSERT((AOS_MSG_BODY % AOS_MSG_BLOCK) == 0,
//              "Message bodies must be a multiple of 32B.");

#define AOS_MSG_MAX (AOS_MSG_BODY + AOS_MSG_HDR)

#define AOS_MSG_WORDS (AOS_MSG_MAX / sizeof(uintptr_t))

/* Any variable-length (string) portion of the message runs from the end of
 * any fixed payload section till the end of the message - this lets us
 * truncate the message for short strings.  We reserve one block for
 * (possible) fixed payload, which lets us define a uniform maximum string
 * length for all message types. */
#define AOS_MSG_MAX_STRING (AOS_MSG_BODY - AOS_MSG_BLOCK)

union aos_msg_hdr {
    struct {
        /* enum aos_msgtype */
        uintptr_t msgtype  : 8;
        /* The length of the message (including this header). */
        uintptr_t length   : 54;
        /* Part of a fragmented message. */
        uintptr_t fragment : 1;
        /* The last (or only) fragment. */
        uintptr_t last     : 1;
    } f;
    uintptr_t raw;
};

STATIC_ASSERT_SIZEOF(union aos_msg_hdr, AOS_MSG_HDR);

struct aos_msg_raw {
    union aos_msg_hdr header;

    uintptr_t body[AOS_MSG_WORDS - 1];
};

STATIC_ASSERT_SIZEOF(struct aos_msg_raw, AOS_MSG_MAX);

struct aos_msg_empty {
    union aos_msg_hdr header;
};

STATIC_ASSERT_SIZEOF(struct aos_msg_empty, AOS_MSG_HDR);

struct aos_msg_number {
    union aos_msg_hdr header;

    uint32_t number;
    uintptr_t padding[2];
};

STATIC_ASSERT_SIZEOF(struct aos_msg_number, AOS_MSG_HDR + AOS_MSG_BLOCK);

struct aos_msg_string {
    union aos_msg_hdr header;

    char string[AOS_MSG_MAX_STRING];
};

STATIC_ASSERT_SIZEOF(struct aos_msg_string, AOS_MSG_MAX - AOS_MSG_BLOCK);

struct aos_msg_errno {
    union aos_msg_hdr header;

    errval_t err;

    uintptr_t padding[2];
};

STATIC_ASSERT_SIZEOF(struct aos_msg_errno, AOS_MSG_HDR + AOS_MSG_BLOCK);

struct aos_msg_domain_exit {
    union aos_msg_hdr header;

    int32_t exit_code;

    uintptr_t padding[2];
};

STATIC_ASSERT_SIZEOF(struct aos_msg_domain_exit, AOS_MSG_HDR + AOS_MSG_BLOCK);

struct aos_msg_ramalloc {
    union aos_msg_hdr header;

    size_t bytes;
    size_t align;

    uintptr_t padding[1];
};

STATIC_ASSERT_SIZEOF(struct aos_msg_ramalloc, AOS_MSG_HDR + AOS_MSG_BLOCK);

struct aos_msg_capalloc_reply {
    union aos_msg_hdr header;

    errval_t err;
    uint32_t size;

    uintptr_t padding[1];
};

STATIC_ASSERT_SIZEOF(struct aos_msg_capalloc_reply,
                     AOS_MSG_HDR + AOS_MSG_BLOCK);

struct aos_msg_devalloc {
    union aos_msg_hdr header;

    uint32_t paddr;
    size_t size;

    uintptr_t padding[1];
};

STATIC_ASSERT_SIZEOF(struct aos_msg_devalloc, AOS_MSG_HDR + AOS_MSG_BLOCK);

struct aos_msg_newdom {
    union aos_msg_hdr header;

    coreid_t core;

    uintptr_t padding[2];

    char cmdline[AOS_MSG_MAX_STRING];
};

STATIC_ASSERT_SIZEOF(struct aos_msg_newdom, AOS_MSG_MAX);

struct aos_msg_newdom_reply {
    union aos_msg_hdr header;

    errval_t err;
    domainid_t pid;

    uintptr_t padding[1];
};

STATIC_ASSERT_SIZEOF(struct aos_msg_newdom_reply, AOS_MSG_HDR + AOS_MSG_BLOCK);

struct aos_msg_domwait {
    union aos_msg_hdr header;

    domainid_t pid;

    uintptr_t padding[2];
};
STATIC_ASSERT_SIZEOF(struct aos_msg_domwait, AOS_MSG_HDR + AOS_MSG_BLOCK);

struct aos_msg_domwait_reply {
    union aos_msg_hdr header;

    int exitcode;
    errval_t status;

    uintptr_t padding[1];
};
STATIC_ASSERT_SIZEOF(struct aos_msg_domwait_reply, AOS_MSG_HDR + AOS_MSG_BLOCK);

struct aos_msg_term_write {
    union aos_msg_hdr header;

    char c;

    uintptr_t padding[2];
};

STATIC_ASSERT_SIZEOF(struct aos_msg_term_write, AOS_MSG_HDR + AOS_MSG_BLOCK);

struct aos_msg_term_read_reply {
    union aos_msg_hdr header;

    errval_t err;
    char c;

    uintptr_t padding[1];
};

STATIC_ASSERT_SIZEOF(struct aos_msg_term_read_reply,
                     AOS_MSG_HDR + AOS_MSG_BLOCK);

struct aos_msg_pid_name {
    union aos_msg_hdr header;

    domainid_t pid;

    uintptr_t padding[2];
};

STATIC_ASSERT_SIZEOF(struct aos_msg_pid_name, AOS_MSG_HDR + AOS_MSG_BLOCK);

struct aos_msg_pid_name_reply {
    union aos_msg_hdr header;

    errval_t err;

    uintptr_t padding[2];

    char name[AOS_MSG_MAX_STRING];
};

STATIC_ASSERT_SIZEOF(struct aos_msg_pid_name_reply, AOS_MSG_MAX);

struct aos_msg_all_pid {
    union aos_msg_hdr header;

    errval_t err;
    size_t num_pids;

    uintptr_t padding[1];

    domainid_t pids[AOS_MSG_MAX_STRING/sizeof(domainid_t)];
};

STATIC_ASSERT_SIZEOF(struct aos_msg_all_pid, AOS_MSG_MAX);

struct aos_msg_ns_lookup_reply {
    union aos_msg_hdr header;

    errval_t err;
    uintptr_t is_ump;

    uintptr_t padding[1];
};

STATIC_ASSERT_SIZEOF(struct aos_msg_ns_lookup_reply,
                     AOS_MSG_HDR + AOS_MSG_BLOCK);

struct aos_msg_new_chan_request {
    union aos_msg_hdr header;

    uintptr_t is_ump;
    uintptr_t state;

    uintptr_t padding[1];
};

STATIC_ASSERT_SIZEOF(struct aos_msg_new_chan_request,
                     AOS_MSG_HDR + AOS_MSG_BLOCK);

struct aos_msg_new_chan_reply {
    union aos_msg_hdr header;

    errval_t err;
    uintptr_t state;
    uintptr_t is_ump;
};

STATIC_ASSERT_SIZEOF(struct aos_msg_new_chan_reply,
                     AOS_MSG_HDR + AOS_MSG_BLOCK);

/* bulk transfers */

struct aos_msg_bulk_read_request {
    union aos_msg_hdr header;

    uintptr_t id;
    uintptr_t count;
    uintptr_t offset;
};

STATIC_ASSERT_SIZEOF(struct aos_msg_bulk_read_request,
                     AOS_MSG_HDR + AOS_MSG_BLOCK);

struct aos_msg_bulk_reply {
    union aos_msg_hdr header;

    errval_t err;
    uintptr_t id;
    uintptr_t count;
};

STATIC_ASSERT_SIZEOF(struct aos_msg_bulk_reply,
                     AOS_MSG_HDR + AOS_MSG_BLOCK);

// TODO GET THIS RIGHT! (changed id + count from uinptr_t to uint32_t)
struct aos_msg_bulk_write_request {
    union aos_msg_hdr header;

    uint32_t id;
    uint32_t count;
    uintptr_t offset;
    uintptr_t sync;
};

STATIC_ASSERT_SIZEOF(struct aos_msg_bulk_write_request,
                     AOS_MSG_HDR + AOS_MSG_BLOCK);

struct aos_msg_bulk_setup_response {
    union aos_msg_hdr header;

    errval_t err;

    uintptr_t padding[2];
};

STATIC_ASSERT_SIZEOF(struct aos_msg_bulk_setup_response,
                     AOS_MSG_HDR + AOS_MSG_BLOCK);

struct aos_msg_bulk_setup_request {
    union aos_msg_hdr header;

    uintptr_t padding[3];
};

STATIC_ASSERT_SIZEOF(struct aos_msg_bulk_setup_request,
                     AOS_MSG_HDR + AOS_MSG_BLOCK);

struct aos_msg_del_revoke_request {
    union aos_msg_hdr header;
    capaddr_t src;
    uint8_t level;
    uintptr_t padding[2];
};

STATIC_ASSERT_SIZEOF(struct aos_msg_del_revoke_request,
                     AOS_MSG_HDR + AOS_MSG_BLOCK);

struct aos_msg_retype_request {
    union aos_msg_hdr header;
    capaddr_t src_root;
    capaddr_t src;
    capaddr_t dest_root;
    capaddr_t dest_cn;
    cslot_t   dest_slot;
    // XXX: this works but is not actually correct!
    uint32_t  src_offset;
    uint32_t  objsize;
    uint32_t  count;
    enum objtype newtype;
    uint8_t   src_level;
    uint8_t   destcn_level;
    uintptr_t padding[1];
};

STATIC_ASSERT_SIZEOF(struct aos_msg_retype_request,
                     AOS_MSG_HDR + 2*AOS_MSG_BLOCK);

struct aos_msg_distops_reply {
    union aos_msg_hdr header;
    errval_t status;
    uintptr_t padding[2];
};

STATIC_ASSERT_SIZEOF(struct aos_msg_distops_reply,
                     AOS_MSG_HDR + AOS_MSG_BLOCK);

struct aos_msg_ram_transfer_reply {
    union aos_msg_hdr header;

    uint32_t base;
    uint32_t bytes;

    uintptr_t padding[2];
};

STATIC_ASSERT_SIZEOF(struct aos_msg_ram_transfer_reply,
                     AOS_MSG_HDR + AOS_MSG_BLOCK);

struct aos_msg_ramfs {
    union aos_msg_hdr header;

    char name[AOS_MSG_MAX_STRING];
};

STATIC_ASSERT_SIZEOF(struct aos_msg_ramfs, AOS_MSG_MAX - AOS_MSG_BLOCK);

struct aos_msg_get_bootinfo_reply {
    union aos_msg_hdr header;

    uint32_t base;
    uint32_t bytes;
    uint32_t strings_base;
    uint32_t strings_bytes;

    uintptr_t padding[1];
};

STATIC_ASSERT_SIZEOF(struct aos_msg_get_bootinfo_reply,
                     AOS_MSG_HDR + AOS_MSG_BLOCK);

struct aos_msg_sdma_fill_request {
    union aos_msg_hdr header;

    uintptr_t pattern;

    uintptr_t padding[2];
};

STATIC_ASSERT_SIZEOF(struct aos_msg_sdma_fill_request,
                     AOS_MSG_HDR + AOS_MSG_BLOCK);

#endif // _LIB_AOS_AOS_MSG_DEF_H
