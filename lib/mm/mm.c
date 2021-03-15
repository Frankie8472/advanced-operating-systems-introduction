/**
 * \file
 * \brief A library for managing physical memory (i.e., caps)
 */

#include <mm/mm.h>
#include <aos/debug.h>
#include <aos/solution.h>


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

void mm_destroy(struct mm *mm)
{
    assert(!"NYI");
}

/**
 * \brief Allocates aligned memory in the form of a RAM capability
 *
 * \param mm Pointer to MM allocator instance data
 * \param size Amount of RAM to allocate, in bytes
 * \param alignment Alignment of RAM to allocate slot used for the cap in #ret, if any
 * \param ret Pointer to capref struct, filled-in with allocated cap location
 */
errval_t mm_add(struct mm *mm, struct capref cap, genpaddr_t base, size_t size)
{
    if (mm == NULL){
        return MM_ERR_MM_ADD;
    }

    struct capinfo capinfo_new;
    capinfo_new.cap = cap;
    capinfo_new.base = base;
    capinfo_new.size = (genpaddr_t) size;

    struct mmnode* mmnode_new = slab_alloc(&(mm->slabs));
    mmnode_new->type = NodeType_Free;
    mmnode_new->cap = capinfo_new;
    mmnode_new->prev = NULL;
    mmnode_new->next = NULL;
    mmnode_new->base = base;
    mmnode_new->size = (gensize_t) size; // Control type

    if (mm->head == NULL){
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
    if (mm == NULL){
        return LIB_ERR_SHOULD_NOT_GET_HERE;
    }

    // Search free matching node
    struct mmnode* cm;
    for (cm = mm->head; cm != NULL && (cm->type != NodeType_Free || cm->size < size); cm = cm->next);

    if (cm == NULL){
        return MM_ERR_SLOT_MM_ALLOC;
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

    if (err_is_fail(err)){
        return SYS_ERR_RETYPE_CREATE;
    }
    return SYS_ERR_OK;
}

errval_t mm_alloc(struct mm *mm, size_t size, struct capref *retcap)
{
    return mm_alloc_aligned(mm, size, BASE_PAGE_SIZE, retcap);
}


errval_t mm_free(struct mm *mm, struct capref cap, genpaddr_t base, gensize_t size)
{
    // Find the node on that address
    struct mmnode* cm;
    for (cm = mm->head; cm != NULL && cm->base != base; cm = cm->next);

    // Handle error if the node is not found on that address
    if (cm == NULL){
        return MM_ERR_MM_FREE;
    }

    // Free node
    assert(cm->size == size);
    cm->type = NodeType_Free;

    // Fusion with possible free predecessor
    if (cm->prev != NULL && cm->prev->type == NodeType_Free){
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
    if (cm->next != NULL && cm->next->type == NodeType_Free){
        cm->size = cm->size + cm->next->size;
        if (cm->next->next != NULL){
            cm->next = cm->next->next;
            slab_free(&(mm->slabs), cm->next->prev);
            cm->next->prev = cm;
        } else {
            slab_free(&(mm->slabs), cm->next);
            cm->next = NULL;
        }
    }

    errval_t err = cap_destroy(cap);
    if (err_is_fail(err)){
        return LIB_ERR_CAP_DESTROY;
    }
    return SYS_ERR_OK;
}
