/**
 * \file
 * \brief PMAP Implementaiton for AOS
 */

/*
 * Copyright (c) 2019 ETH Zurich.
 * All rights reserved.
 *
 * This file is distributed under the terms in the attached LICENSE file.
 * If you do not find this file, copies can be found by writing to:
 * ETH Zurich D-INFK, Universitaetstr. 6, CH-8092 Zurich. Attn: Systems Group.
 */

#ifndef PAGING_TYPES_H_
#define PAGING_TYPES_H_ 1

#define MCN_COUNT DIVIDE_ROUND_UP(PTABLE_ENTRIES, L2_CNODE_SLOTS)

#define PMAP_PREALLOC_PTABLE_SLABS 8

#define PMAP_META_SIZE ROUND_UP(SLAB_STATIC_SIZE(2048, sizeof(struct ptable)), BASE_PAGE_SIZE)


#define VADDR_OFFSET ((lvaddr_t)512UL*1024*1024*1024) // 1GB

#define PAGING_SLAB_BUFSIZE 32

#define VREGION_FLAGS_READ     0x01 // Reading allowed
#define VREGION_FLAGS_WRITE    0x02 // Writing allowed
#define VREGION_FLAGS_EXECUTE  0x04 // Execute allowed
#define VREGION_FLAGS_NOCACHE  0x08 // Caching disabled
#define VREGION_FLAGS_MPB      0x10 // Message passing buffer
#define VREGION_FLAGS_GUARD    0x20 // Guard page
#define VREGION_FLAGS_MASK     0x2f // Mask of all individual VREGION_FLAGS

#define VREGION_FLAGS_READ_WRITE \
    (VREGION_FLAGS_READ | VREGION_FLAGS_WRITE)
#define VREGION_FLAGS_READ_EXECUTE \
    (VREGION_FLAGS_READ | VREGION_FLAGS_EXECUTE)
#define VREGION_FLAGS_READ_WRITE_NOCACHE \
    (VREGION_FLAGS_READ | VREGION_FLAGS_WRITE | VREGION_FLAGS_NOCACHE)
#define VREGION_FLAGS_READ_WRITE_MPB \
    (VREGION_FLAGS_READ | VREGION_FLAGS_WRITE | VREGION_FLAGS_MPB)

typedef int paging_flags_t;

struct ptable {
    struct ptable *parent;
    struct capref ptablecap;

    struct ptable *children[PTABLE_ENTRIES];

    struct capref mcn[MCN_COUNT];       ///< CNodes to store mappings (caprefs)
    struct cnoderef mcnode[MCN_COUNT];  ///< CNodeRefs of mapping cnodes
    uint16_t entry;                     ///< the entry in the parent
    enum objtype type;
};

struct paging_region {
    paging_flags_t flags;
    lvaddr_t base_addr;
    lvaddr_t current_addr;
    size_t region_size;
    struct paging_region* next;
    // TODO: if needed add struct members for tracking state
};

struct pmap {
    struct slot_allocator *slot_alloc;   ///< slot allocator for vnodes
    struct slab_allocator ptable_slabs;  ///< slab allocator for vndoes

    struct paging_region meta;

    struct ptable root;  ///< the root of the page-table

    struct ptable ptablebuf[PMAP_PREALLOC_PTABLE_SLABS + 1];

    lvaddr_t max_mappable_addr;
    lvaddr_t min_mappable_addr;
};

// struct to store the paging status of a process
struct paging_state {
    lvaddr_t vaddr_start;

    // list of paging_regoin
    struct paging_region* head;

    // root
    struct capref root;
    struct slot_allocator *slot_alloc;

    // infrastructure for paging regions
    struct slab_allocator region_alloc;
    struct paging_region region_buf[PAGING_SLAB_BUFSIZE];

    // map for mappings
    struct pmap pmap;

    // page fault handling ram
    struct capref current_ram;
    gensize_t current_ram_size;
    gensize_t current_ram_offset;

    struct thread_mutex pfmutex;
};


#endif  /// PAGING_TYPES_H_
