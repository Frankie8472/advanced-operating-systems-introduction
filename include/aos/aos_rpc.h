/**
 * \file
 * \brief RPC Bindings for AOS
 */

/*
 * Copyright (c) 2013-2016, ETH Zurich.
 * All rights reserved.
 *
 * This file is distributed under the terms in the attached LICENSE file.
 * If you do not find this file, copies can be found by writing to:
 * ETH Zurich D-INFK, Universitaetstr. 6, CH-8092 Zurich. Attn: Systems Group.
 */

#ifndef _LIB_BARRELFISH_AOS_MESSAGES_H
#define _LIB_BARRELFISH_AOS_MESSAGES_H

#include <aos/aos.h>
#include <aos/aos_lmp.h>
#include <aos/aos_ump.h>
#include <aos/aos_msg_def.h>

enum aos_rpc_transport {
    AOS_RPC_LMP,
    AOS_RPC_UMP,
    AOS_RPC_LOCAL,
};

typedef void (*local_rpc_handler_t)(void *chan, void *tx, struct capref tx_cap);

/* An RPC binding, which may be transported over LMP or UMP. */
struct aos_rpc {
    /* This RPC binding may be carried over LMP or UMP. */
    enum aos_rpc_transport transport;

    /* LMP-specific state. */
    struct aos_chan *aos_chan;
    struct waitset ws;

    /* UMP-specific state. */
    struct urpc_pair urpc_pair;

    /* Local-specific state */
    local_rpc_handler_t rpc_handler;

    /* Synchronisation with the LMP handler. */
    struct thread_mutex rpc_lock;
    bool reply_present;

    /* Continuation state for the LMP and local handler. */
    void *rx_ptr;
    struct capref rx_cap;
};

/**
 * \brief Initialize an RPC binding using Local (loopback) transport.
 */
errval_t aos_rpc_init_local(struct aos_rpc *rpc, local_rpc_handler_t rpc_handler);

/**
 * \brief Initialize an RPC binding using LMP transport.
 */
errval_t aos_rpc_init_lmp(struct aos_rpc *rpc, struct aos_chan *chan);

/**
 * \brief Initialize an RPC binding using UMP transport.
 */
errval_t aos_rpc_init_ump(struct aos_rpc *rpc, struct urpc_pair *urpc_pair);

/**
 * \brief "raw" RPC interface, so we can do raw RPCs from code that knows what
 *        it is doing (e.g. distops)
 * \arg rpc the RPC channel to use
 * \arg tx a pre-marshalled message to transmit
 * \arg rx a pre-populated message struct in which to receive
 * \arg tx_cap capref to send
 * \arg rx_cap capref to empty slot for receive
 */
errval_t aos_rpc(struct aos_rpc *rpc, void *tx, void *rx,
                 struct capref tx_cap, struct capref rx_cap);

/**
 * \brief "raw" RPC reply interface, so we can do raw RPC replies from code
 *              that knows what it is doing (e.g. distops)
 * \arg transport the RPC transport type to use (LMP, UMP)
 * \arg channel the channel on which to reply
 * \arg tx a pre-marshalled reply to transmit
 * \arg tx_cap reply capability
 */
errval_t aos_rpc_reply(enum aos_rpc_transport transport, void *channel, void *tx,
                       struct capref tx_cap);

/* XXX - these aren't RPCs! */

/**
 * \brief Send a number.
 */
errval_t aos_rpc_send_number(struct aos_rpc *chan, uintptr_t val);

/**
 * \brief Send a string.
 */
errval_t aos_rpc_send_string(struct aos_rpc *chan, const char *string);

errval_t aos_errno_reply(enum aos_rpc_transport transport, void *channel,
                         errval_t err);

/**
 * \brief Request a RAM capability with >= request_bits of size over the given
 * channel.
 */
errval_t aos_rpc_get_ram_cap(struct aos_rpc *chan, size_t bytes,
                             size_t alignment, struct capref *retcap,
                             size_t *ret_bytes);

errval_t aos_get_ram_cap_reply(enum aos_rpc_transport transport, void *channel,
                               struct capref ram, uintptr_t size,
                               errval_t error);

/**
 * \brief Get one character from the serial port
 */
errval_t aos_rpc_serial_getchar(struct aos_rpc *chan, char *retc);

errval_t aos_serial_getchar_reply(enum aos_rpc_transport transport,
                                  void *channel, const char c, errval_t err);

/**
 * \brief Send one character to the serial port
 */
errval_t aos_rpc_serial_putchar(struct aos_rpc *chan, char c);

/**
 * \brief Request that the process manager start a new process
 * \arg name the name of the process that needs to be spawned (without a
 *           path prefix)
 * \arg newpid the process id of the newly-spawned process
 */
errval_t aos_rpc_process_spawn(struct aos_rpc *chan, char *name,
                               coreid_t core, domainid_t *newpid);

errval_t aos_process_spawn_reply(enum aos_rpc_transport transport,
                                 void *channel, errval_t err, domainid_t pid);

/**
 * \brief Wait for process with PID `pid` to exit, return its exitcode.
 * \arg pid the PID to wait for
 * \arg exitcode the exitcode of process with PID pid
 */
errval_t aos_rpc_process_wait(struct aos_rpc *rpc, domainid_t pid,
                              int *exitcode);

errval_t aos_process_wait_reply(enum aos_rpc_transport transport,
                                void *channel, int exitcode,
                                errval_t waitstatus);

/**
 * \brief Get name of process with the given PID.
 * \arg pid the process id to lookup
 * \arg name A null-terminated character array with the name of the process
 * that is allocated by the rpc implementation. Freeing is the caller's
 * responsibility.
 */
errval_t aos_rpc_process_get_name(struct aos_rpc *chan, domainid_t pid,
                                  char **name);

errval_t aos_process_get_name_reply(enum aos_rpc_transport transport,
                                    void *channel, const char *cmdline,
                                    errval_t err);

/**
 * \brief Get PIDs of all running processes.
 * \arg pids An array containing the process ids of all currently active
 * processes. Will be allocated by the rpc implementation. Freeing is the
 * caller's  responsibility.
 * \arg pid_count The number of entries in `pids' if the call was successful
 */
errval_t aos_rpc_process_get_all_pids(struct aos_rpc *chan,
                                      domainid_t **pids, size_t *pid_count);

errval_t aos_process_get_all_pids_reply(enum aos_rpc_transport transport,
                                        void *channel, domainid_t *pids,
                                        size_t len, errval_t err);

/**
 * \brief Request a device cap for the given region.
 * @param chan  the rpc channel
 * @param paddr physical address of the device
 * @param bytes number of bytes of the device memory
 * @param frame returned frame
 */
errval_t aos_rpc_get_device_cap(struct aos_rpc *chan,
                                lpaddr_t paddr, size_t bytes,
                                struct capref *frame);

errval_t aos_get_device_cap_reply(enum aos_rpc_transport transport,
                                  void *channel, struct capref dev,
                                  uintptr_t size, errval_t error);

/**
 * \brief Wait for the given process to exit.
 * \arg pid of the process to wait on
 * \arg code contains the process' exit code
 */
errval_t aos_rpc_waitpid(struct aos_rpc *chan, domainid_t pid, int32_t *code);


errval_t aos_rpc_nameservice_register(struct aos_rpc *chan, const char *name);

errval_t aos_rpc_nameservice_lookup(struct aos_rpc *chan, const char *name,
                                    struct capref *ep, bool *is_ump);

errval_t aos_send_ns_lookup_reply(enum aos_rpc_transport transport,
                                  void *channel, errval_t error,
                                  bool is_ump, struct capref ep);

/*
 * Bulk Transfer RPCs
 */

#define AOS_RPC_BULK_BLOCK_SIZE 512

errval_t aos_rpc_bulk_setup(struct aos_rpc *chan, struct capref cap);

errval_t aos_rpc_bulk_read(struct aos_rpc *chan, size_t id,
                           size_t count, uint8_t *data);
errval_t aos_rpc_bulk_write(struct aos_rpc *chan, size_t id,
                            size_t count, bool sync, uint8_t *data);
errval_t aos_send_bulk_reply(enum aos_rpc_transport transport, void *channel,
                             errval_t err, size_t id, size_t count);

/*
 * Distops RPCs
 */
errval_t aos_rpc_delete(struct aos_rpc *rpc, struct capref root,
                        capaddr_t src, uint8_t level);
errval_t aos_rpc_revoke(struct aos_rpc *rpc, struct capref root,
                        capaddr_t src, uint8_t level);
errval_t aos_rpc_retype(struct aos_rpc *rpc, struct capref src_root,
                        struct capref dest_root, capaddr_t src,
                        gensize_t offset, enum objtype new_type,
                        gensize_t objsize, size_t count, capaddr_t dest_cnode,
                        capaddr_t dest_slot, uint8_t dest_level);

/*
 * RPCs Between Init Processes
 */
errval_t aos_rpc_ram_transfer(struct aos_rpc *rpc, lpaddr_t *base,
                              size_t *bytes);
errval_t aos_ram_transfer_reply(enum aos_rpc_transport transport,
                                void *channel, lpaddr_t base, size_t bytes);

errval_t aos_rpc_get_bootinfo(struct aos_rpc *rpc, lpaddr_t *base,
                              size_t *bytes, lpaddr_t *strings_base,
                              size_t *strings_bytes);
errval_t aos_get_bootinfo_reply(enum aos_rpc_transport transport,
                                void *channel, lpaddr_t base, size_t bytes,
                                lpaddr_t strings_base, size_t strings_bytes);

/*
 * Communication (synchronous) with the SDMA engine.
 */
errval_t aos_rpc_sdma_copy(struct aos_rpc *rpc, struct capref dest, struct capref source);
errval_t aos_rpc_sdma_fill(struct aos_rpc *rpc, struct capref dest, uint8_t pattern);


/**
 * \brief Returns the RPC channel to init.
 */
struct aos_rpc *aos_rpc_get_init_channel(void);

/**
 * \brief Returns the channel to the memory server
 */
struct aos_rpc *aos_rpc_get_memory_channel(void);

/**
 * \brief Returns the channel to the process manager
 */
struct aos_rpc *aos_rpc_get_process_channel(void);

/**
 * \brief Returns the channel to the serial console
 */
struct aos_rpc *aos_rpc_get_serial_channel(void);

#endif // _LIB_BARRELFISH_AOS_MESSAGES_H
