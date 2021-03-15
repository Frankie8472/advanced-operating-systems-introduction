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

    return LIB_ERR_NOT_IMPLEMENTED;
}

void mm_destroy(struct mm *mm)
{
    assert(!"NYI");
}

errval_t mm_add(struct mm *mm, struct capref cap, genpaddr_t base, size_t size)
{
    struct capinfo* capinfo_new;
    capinfo_new->cap = cap;
    capinfo_new->base = base;
    capinfo_new->size = size;

    struct mmnode* mmnode_new;
    mmnode_new->type = NodeType_Free;
    mmnode_new->cap = *capinfo_new;
    mmnode_new->prev = NULL;
    mmnode_new->next = NULL;
    mmnode_new->base = base;
    mmnode_new->size = size;

    if (mm->head == NULL){
        mm->head = mmnode_new;
    } else {
        mmnode_new->next = mm->head;
        mm->head = mmnode_new;
    }
    return LIB_ERR_NOT_IMPLEMENTED;
}


errval_t mm_alloc_aligned(struct mm *mm, size_t size, size_t alignment, struct capref *retcap)
{
    return LIB_ERR_NOT_IMPLEMENTED;
}

errval_t mm_alloc(struct mm *mm, size_t size, struct capref *retcap)
{
    return mm_alloc_aligned(mm, size, BASE_PAGE_SIZE, retcap);
}


errval_t mm_free(struct mm *mm, struct capref cap, genpaddr_t base, gensize_t size)
{
    return LIB_ERR_NOT_IMPLEMENTED;
}
