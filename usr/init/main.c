/**
 * \file
 * \brief init process for child spawning
 */

/*
 * Copyright (c) 2007, 2008, 2009, 2010, 2016, ETH Zurich.
 * All rights reserved.
 *
 * This file is distributed under the terms in the attached LICENSE file.
 * If you do not find this file, copies can be found by writing to:
 * ETH Zurich D-INFK, Haldeneggsteig 4, CH-8092 Zurich. Attn: Systems Group.
 */

#include <stdio.h>
#include <stdlib.h>

#include <aos/aos.h>
#include <aos/morecore.h>
#include <aos/paging.h>
#include <aos/waitset.h>
#include <aos/aos_rpc.h>
#include <mm/mm.h>
#include <grading.h>

#include "mem_alloc.h"




struct bootinfo *bi;

coreid_t my_core_id;


static void test(void) {
    debug_printf("== START Testing ==\n");
    size_t l = 3;
    struct capref cap[l];

    for (int i= 0; i < l; i++) {
        //debug_printf("== %d\n", i);
        ram_alloc_aligned(&cap[i], 4096, 1);
    }
    debug_printf(">> Allocate capabilities (4096, 1): OK \n");


    for (int i= 0; i < l; i++) {
        //debug_printf("== %d\n", i);
        aos_ram_free(cap[i]);
    }
    debug_printf(">> Freeing capabilities (4096, 1): OK \n");

    for (int i= 0; i < l; i++) {
        ram_alloc_aligned(&cap[i], 8192, 2);
    }
    debug_printf(">> Allocate capabilities (8192, 2): OK \n");

    for (int i = 0; i < l/4; i+=4) {
        aos_ram_free(cap[i]);
        aos_ram_free(cap[i+1]);
        aos_ram_free(cap[i+2]);
        aos_ram_free(cap[i+3]);
    }
    debug_printf(">> Freeing capabilities (8192, 2): OK \n");


    struct capref frame;
    size_t size;
    frame_alloc(&frame, 4096, &size);

    debug_printf(">> Frame allocating (4096, %zu): OK \n", size);

    lvaddr_t addr = VADDR_OFFSET + 0x10000;
    paging_map_fixed_attr(get_current_paging_state(), addr, frame, 4096, VREGION_FLAGS_READ_WRITE);

    debug_printf(">> Assign one VA to one frame: OK \n");

    l = 49; // No returns on > 30, investigate!
    struct capref frames[l];
    for (int i = 0; i < l; i++) {
        frame_alloc(&frames[i], 4096, &size);
        paging_map_fixed_attr(get_current_paging_state(), VADDR_OFFSET + 0x10000 + 0x1000*(i+1), frames[i], 4096, VREGION_FLAGS_READ_WRITE);
    }
    debug_printf(">> Multi Frame allocating (4096, %zu): OK \n", size);
    debug_printf(">> Multi Assign one VA to one frame: OK \n");

    int64_t * var = (int64_t *) addr;

    for (int i = 0; i < 512; i++) {
        var[i] = 42; // 512 int64
    }

    for (int i = 0; i < 512; i+=2) {
        var[i] += var[i+1]; // 512 int64
    }
    debug_printf(">> Calculations on allocated space: OK \n");
    debug_printf("== END Testing ==\n");
}


static int
bsp_main(int argc, char *argv[])
{
    errval_t err;

    // Grading 
    grading_setup_bsp_init(argc, argv);

    // First argument contains the bootinfo location, if it's not set
    bi = (struct bootinfo*)strtol(argv[1], NULL, 10);
    assert(bi);

    err = initialize_ram_alloc();
    if(err_is_fail(err)){
        DEBUG_ERR(err, "initialize_ram_alloc");
    }

    // TODO: initialize mem allocator, vspace management here

    test();

    // Grading 
    grading_test_early();

    // TODO: Spawn system processes, boot second core etc. here
    
    // Grading 
    grading_test_late();

    debug_printf("Message handler loop\n");
    // Hang around
    struct waitset *default_ws = get_default_waitset();
    while (true) {
        err = event_dispatch(default_ws);
        if (err_is_fail(err)) {
            DEBUG_ERR(err, "in event_dispatch");
            abort();
        }
    }

    return EXIT_SUCCESS;
}

static int
app_main(int argc, char *argv[]) {
    // Implement me in Milestone 5
    // Remember to call
    // - grading_setup_app_init(..);
    // - grading_test_early();
    // - grading_test_late();
    return LIB_ERR_NOT_IMPLEMENTED;
}



int main(int argc, char *argv[])
{
    errval_t err;


    /* Set the core id in the disp_priv struct */
    err = invoke_kernel_get_core_id(cap_kernel, &my_core_id);
    assert(err_is_ok(err));
    disp_set_core_id(my_core_id);

    debug_printf("init: on core %" PRIuCOREID ", invoked as:", my_core_id);
    for (int i = 0; i < argc; i++) {
       printf(" %s", argv[i]);
    }
    printf("\n");
    fflush(stdout);


    if(my_core_id == 0) return bsp_main(argc, argv);
    else                return app_main(argc, argv);
}
