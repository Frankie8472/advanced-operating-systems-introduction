/*
 * Copyright (c) 2016, ETH Zurich.
 *
 * This file is distributed under the terms in the attached LICENSE file.  If
 * you do not find this file, copies can be found by writing to: ETH Zurich
 * D-INFK, Universitaetstr. 6, CH-8092 Zurich. Attn: Systems Group.
 */

#ifndef __KERNEL_CAP_INVOCATIONS
#define __KERNEL_CAP_INVOCATIONS

#include <aos/aos.h>

/**
 * \brief Spawn a new core.
 *
 * \param core_id    APIC ID of the core to try booting
 * \param cpu_type   Barrelfish target CPU type
 * \param core_data  Address of struct core_data for new core in kernel-virtual memory
 */
static inline errval_t
invoke_monitor_spawn_core(coreid_t core_id, enum cpu_type cpu_type,
                          forvaddr_t core_data)
{
    return cap_invoke5(cap_kernel, IPICmd_Send_Start, core_id, cpu_type,
            (uintptr_t)(core_data >> 32), (uintptr_t) core_data).error;
}

#endif /* __KERNEL_CAP_INVOCATIONS */
