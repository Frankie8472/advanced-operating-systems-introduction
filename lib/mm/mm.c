/**
 * \file
 * \brief A library for managing physical memory (i.e., caps)
 */

#include <mm/mm.h>
#include <aos/debug.h>
#include <aos/solution.h>

/**
 * \brief Initializes an mmnode to the given MM allocator instance data
 *
 * \param mm Pointer to MM allocator instance data
 * \param objtype Type of the allocator and its nodes
 * \param slab_refill_func Pointer to function to call when out of memory (or NULL)
 * \param slot_alloc_func Slot allocator for allocating cspace
 * \param slot_refill_func Slot allocator refill function
 * \param slot_alloc_inst Opaque instance pointer for slot allocator
 */
errval_t mm_init(struct mm *mm, enum objtype objtype,
                     slab_refill_func_t slab_refill_func,
                     slot_alloc_t slot_alloc_func,
                     slot_refill_t slot_refill_func,
                     void *slot_alloc_inst)
{
    slab_init(&(mm->slabs), sizeof(struct mmnode), slab_refill_func);
    mm->slot_alloc = slot_alloc_func;
    mm->slot_refill = slot_refill_func;
    mm->slot_alloc_inst = slot_alloc_inst;
    mm->objtype = objtype;
    mm->head = NULL;

    return SYS_ERR_OK;
}

/**
 * \brief Destroys an MM allocator instance data with all its nodes and capabilities
 *
 * \param mm Pointer to MM allocator instance data
 */
void mm_destroy(struct mm *mm)
{
    struct mmnode *cm, *tm;
    for (cm = mm->head; cm != NULL; cm = tm) {
        for (tm = cm->next; tm != NULL && capcmp(cm->cap.cap, tm->cap.cap); tm = cm->next) {
            cm->next = tm->next;
            slab_free(&(mm->slabs), tm);
        }
        cap_revoke(cm->cap.cap);
        cap_destroy(cm->cap.cap);
        slab_free(&(mm->slabs), cm);
    }
}

/**
 * \brief Adds an mmnode to the given MM allocator instance data
 *
 * \param mm Pointer to MM allocator instance data
 * \param cap Capability to the given RAM space
 * \param base Start_add of the RAM to allocate
 * \param size Amount of RAM to allocate, in bytes
 */
errval_t mm_add(struct mm *mm, struct capref cap, genpaddr_t base, size_t size)
{
    if (mm == NULL) {
        return MM_ERR_MM_ADD;
    }

    struct capinfo capinfo_new;
    capinfo_new.cap = cap;
    capinfo_new.base = base;
    capinfo_new.size = (genpaddr_t) size;

    if (slab_freecount(&(mm->slabs)) < 2) {
        slab_default_refill(&(mm->slabs));
    }

    struct mmnode* mmnode_new = slab_alloc(&(mm->slabs));
    mmnode_new->type = NodeType_Free;
    mmnode_new->cap = capinfo_new;
    mmnode_new->prev = NULL;
    mmnode_new->next = NULL;
    mmnode_new->base = base;
    mmnode_new->size = (gensize_t) size; // Control type

    if (mm->head == NULL) {
        mm->head = mmnode_new;
    } else {
        mmnode_new->next = mm->head;
        mm->head = mmnode_new;
    }

    return SYS_ERR_OK;
}

/**
 * \brief Allocates aligned memory in the form of a RAM capability
 *
 * \param mm Pointer to MM allocator instance data
 * \param size Amount of RAM to allocate, in bytes
 * \param alignment Alignment of RAM to allocate slot used for the cap in #ret, if any
 * \param retcap Pointer to capref struct, filled-in with allocated cap location
 */
errval_t mm_alloc_aligned(struct mm *mm, size_t size, size_t alignment, struct capref *retcap)
{
    if (mm == NULL) {
        return LIB_ERR_SHOULD_NOT_GET_HERE;
    }
    // TODO: find out what alignment is used for -> split in three parts with the alignment?

    // Search free matching node
    struct mmnode *cm;
    for (cm = mm->head; cm != NULL && (cm->type != NodeType_Free || cm->size < size); cm = cm->next);

    if (cm == NULL) {
        debug_printf("\nmm_alloc_aligned: cm is null\n");
        return MM_ERR_SLOT_MM_ALLOC;
    }

    static bool isrefill = false;
    if (!isrefill && slab_freecount(&(mm->slabs)) < 6) {
        isrefill = true;
        debug_printf("\nenter: %zu\n", slab_freecount(&(mm->slabs)));
        slab_default_refill(&(mm->slabs));
        debug_printf("\nleave: %zu\n", slab_freecount(&(mm->slabs)));
        isrefill = false;
    }

    // Node fragmentation
    struct mmnode* mmnode_new = slab_alloc(&(mm->slabs));
    mmnode_new->type = NodeType_Allocated;
    mmnode_new->cap = cm->cap;
    mmnode_new->prev = cm->prev;
    mmnode_new->next = cm;
    mmnode_new->base = cm->base;
    mmnode_new->size = (gensize_t) size;

    cm->size = cm->size - (gensize_t) size;
    cm->base = cm->base + (gensize_t) size;
    cm->prev = mmnode_new;

    // Cap fragmentation
    assert(retcap != NULL);
    mm->slot_alloc(mm->slot_alloc_inst, 1, retcap);
    errval_t err = cap_retype(*retcap, cm->cap.cap, mmnode_new->base - cm->cap.base, mm->objtype, (gensize_t) size, 1);

    if (err_is_fail(err)) {
        DEBUG_ERR(err, "mm_alloc_aligned: cap_retype");
        return SYS_ERR_RETYPE_CREATE;
    }
    return SYS_ERR_OK;
}

errval_t mm_alloc(struct mm *mm, size_t size, struct capref *retcap)
{
    return mm_alloc_aligned(mm, size, BASE_PAGE_SIZE, retcap);
}

/**
 * \brief Freeing allocated RAM and associated capability
 *
 * \param mm Pointer to MM allocator instance data
 * \param cap The capability fot the allocated RAM
 * \param base The start_addr for allocated RAM the cap is for
 * \param size The size of the allocated RAM the capability is for
 */
errval_t mm_free(struct mm *mm, struct capref cap, genpaddr_t base, gensize_t size)
{
    // Find the node on that address
    struct mmnode *cm;
    for (cm = mm->head; cm != NULL && cm->base != base; cm = cm->next);

    // Handle error if the node is not found on that address
    if (cm == NULL) {
        return MM_ERR_MM_FREE;
    }

    // Free node
    assert(cm->size == size);
    cm->type = NodeType_Free;

    // Fusion with possible free predecessor
    if (cm->prev != NULL && cm->prev->type == NodeType_Free) {
        cm->size = cm->size + cm->prev->size;
        if (cm->prev->prev != NULL) {
            cm->prev = cm->prev->prev;
            slab_free(&(mm->slabs), cm->prev->next);
            cm->prev->next = cm;
        } else {
            slab_free(&(mm->slabs), cm->prev);
            cm->prev = NULL;
        }
    }

    // Fusion with possible free successor
    if (cm->next != NULL && cm->next->type == NodeType_Free) {
        cm->size = cm->size + cm->next->size;
        if (cm->next->next != NULL) {
            cm->next = cm->next->next;
            slab_free(&(mm->slabs), cm->next->prev);
            cm->next->prev = cm;
        } else {
            slab_free(&(mm->slabs), cm->next);
            cm->next = NULL;
        }
    }

    errval_t err = cap_destroy(cap);
    if (err_is_fail(err)) {
        DEBUG_ERR(err, "mm_alloc_aligned: cap_destroy");
        return LIB_ERR_CAP_DESTROY;
    }
    return SYS_ERR_OK;
}
