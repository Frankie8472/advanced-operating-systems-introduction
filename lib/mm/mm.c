/**
 * \file
 * \brief A library for managing physical memory (i.e., caps)
 */

#include <mm/mm.h>
#include <aos/debug.h>
#include <aos/solution.h>

/**
 * \brief Helper function:
 *        Splits an mmnode_left into two and returns the left block in mmnode_left
 *
 * \param mm Pointer to MM allocator instance data
 * \param mmnode_left Pointer to the mmnode_left to split
 * \param size The size of the first block of the split
 */
static void mmnode_split(struct mmnode *mmnode_left, struct mmnode *mmnode_right, gensize_t size)
{
    mmnode_right->type = mmnode_left->type;
    mmnode_right->cap = mmnode_left->cap;
    mmnode_right->prev = mmnode_left;
    mmnode_right->next = mmnode_left->next;
    mmnode_right->base = mmnode_left->base + size;
    mmnode_right->size = mmnode_left->size - size;

    mmnode_left->size = size;
    mmnode_left->next = mmnode_right;
}

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
    if (mm == NULL) {
        DEBUG_ERR(MM_ERR_NOT_FOUND, "mm.c/mm_init: mm is null");
        return MM_ERR_NOT_FOUND;
    }

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
    errval_t err;
    if (mm == NULL) {
        DEBUG_ERR(MM_ERR_NOT_FOUND, "mm.c/mm_destroy: mm is null");
        return;
    }

    struct mmnode *cm, *nm;
    for (cm = mm->head; cm != NULL; cm = nm) {
        for (nm = cm->next; nm != NULL && capcmp(cm->cap.cap, nm->cap.cap); nm = cm->next) {
            cm->next = nm->next;
            slab_free(&(mm->slabs), nm);
        }
        err = cap_revoke(cm->cap.cap);
        if (err) {
            DEBUG_ERR(LIB_ERR_REMOTE_REVOKE, "mm.c/mm_destroy: cap_revoke error");
            return;
        }

        err = cap_destroy(cm->cap.cap);
        if (err) {
            DEBUG_ERR(LIB_ERR_CAP_DESTROY, "mm.c/mm_destroy: cap_destroy error");
            return;
        }

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
        DEBUG_ERR(MM_ERR_NOT_FOUND, "mm.c/mm_add: mm is null");
        return MM_ERR_NOT_FOUND;
    }

    struct capinfo capinfo_new = {
        .cap = cap,
        .base = base,
        .size = (genpaddr_t) size
    };

    struct mmnode* mmnode_new = slab_alloc(&(mm->slabs));
    mmnode_new->type = NodeType_Free;
    mmnode_new->cap = capinfo_new;
    mmnode_new->prev = NULL;
    mmnode_new->next = NULL;
    mmnode_new->base = base;
    mmnode_new->size = (gensize_t) size;

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
    errval_t err = SYS_ERR_OK;
    struct mmnode *nm = slab_alloc(&(mm->slabs)); // must be called before choosing cm!!!
    // LOCK NM
    if (mm == NULL) {
        DEBUG_ERR(MM_ERR_NOT_FOUND, "mm.c/mm_alloc_aligned: mm is null");
        return MM_ERR_NOT_FOUND;
    }

    // Handle alignment
    struct mmnode *cm = mm->head;
    size_t offset = (alignment - ((cm->base)%alignment))%alignment;
    // Search free matching (large enough) node
    for (; cm != NULL && (cm->type != NodeType_Free || (cm->size - offset) < size); cm = cm->next) {
        offset = (alignment - ((cm->base)%alignment))%alignment;
    }
    // LOCK CM
    if (cm == NULL) {
        DEBUG_ERR(MM_ERR_FIND_NODE, "mm_alloc_aligned: cm is null -> no large enough mmnode found\n");
        return MM_ERR_FIND_NODE;
    }

    // Node fragmentation
    if (offset != 0) {
        mmnode_split(cm, nm, offset);
        // UNLOCK NM + CM
        return mm_alloc_aligned(mm, size, alignment, retcap);
    }

    mmnode_split(cm, nm, size);
    cm->type = NodeType_Allocated;
    // UNLOCK CM + NM

    // Cap fragmentation
    assert(retcap != NULL);
    mm->slot_alloc(mm->slot_alloc_inst, 1, retcap);

    debug_printf("CAP_RETYPING: %lx, %lx\n", (cm->base)-(cm->cap.base), (cm->base)-(cm->cap.base)+size);
    err = cap_retype(*retcap, cm->cap.cap, (cm->base)-(cm->cap.base), mm->objtype, (gensize_t) size, 1);
    if (err_is_fail(err)) {
        DEBUG_ERR(err, "mm.c/mm_alloc_aligned: cap_retype");
        return err_push(err, SYS_ERR_RETYPE_CREATE);
    }
    return err;
}

errval_t mm_alloc(struct mm *mm, size_t size, struct capref *retcap)
{
    return mm_alloc_aligned(mm, size, BASE_PAGE_SIZE, retcap);
}

/**
 * \brief Helper function: Merge free node with its free successor
 *
 * \param mm Pointer to MM allocator instance data
 * \param mmnode Pointer to the mmnode
 */
static void successor_merge(struct mm *mm, struct mmnode *mmnode)
{
    if (mmnode != NULL && mmnode->next != NULL && mmnode->type == NodeType_Free && mmnode->next->type == NodeType_Free && capcmp(mmnode->cap.cap, mmnode->next->cap.cap)) {
        mmnode->size = mmnode->size + mmnode->next->size;
        if (mmnode->next->next != NULL) {
            mmnode->next = mmnode->next->next;
            slab_free(&(mm->slabs), mmnode->next->prev);
            mmnode->next->prev = mmnode;
        } else {
            slab_free(&(mm->slabs), mmnode->next);
            mmnode->next = NULL;
        }
    }
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
    errval_t err = SYS_ERR_OK;
    struct mmnode *cm;
    for (cm = mm->head; cm != NULL && cm->base != base; cm = cm->next);

    if (cm == NULL) {
        DEBUG_ERR(MM_ERR_NOT_FOUND, "mm.c/mm_free: mm is null");
        return MM_ERR_NOT_FOUND;
    }

    // Free node
    assert(cm->size == size);
    cm->type = NodeType_Free;

    // Fusion with possible free predecessor
    successor_merge(mm, cm);
    successor_merge(mm, cm->prev);

    //debug_printf("FREERETYPING: %lx, %lx\n", base-(cm->cap.base), base-(cm->cap.base)+size);
    err = cap_destroy(cap);
    if (err_is_fail(err)) {
        DEBUG_ERR(err, "mm.c/mm_alloc_aligned: cap_destroy");
        return err_push(err, LIB_ERR_CAP_DESTROY);
    }
    return err;
}
