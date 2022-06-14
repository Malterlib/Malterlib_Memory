// Copyright © 2015 Hansoft AB
// Distributed under the MIT license, see license text in LICENSE.Malterlib

#pragma once

typedef struct malloc_introspection_t_10_7 {
    kern_return_t (*enumerator)(task_t task, void *, unsigned type_mask, vm_address_t zone_address, memory_reader_t reader, vm_range_recorder_t recorder); /* enumerates all the malloc pointers in use */
    size_t	(*good_size)(malloc_zone_t *zone, size_t size);
    boolean_t 	(*check)(malloc_zone_t *zone); /* Consistency checker */
    void 	(*print)(malloc_zone_t *zone, boolean_t verbose); /* Prints zone  */
    void	(*log)(malloc_zone_t *zone, void *address); /* Enables logging of activity */
    void	(*force_lock)(malloc_zone_t *zone); /* Forces locking zone */
    void	(*force_unlock)(malloc_zone_t *zone); /* Forces unlocking zone */
    void	(*statistics)(malloc_zone_t *zone, malloc_statistics_t *stats); /* Fills statistics */
    boolean_t   (*zone_locked)(malloc_zone_t *zone); /* Are any zone locks held */

    /* Discharge checking. Present in version >= 7. */
    boolean_t	(*enable_discharge_checking)(malloc_zone_t *zone);
    void	(*disable_discharge_checking)(malloc_zone_t *zone);
    void	(*discharge)(malloc_zone_t *zone, void *memory);
#ifdef __BLOCKS__
    void        (*enumerate_discharged_pointers)(malloc_zone_t *zone, void (^report_discharged)(void *memory, void *info));
#else
    void	*enumerate_unavailable_without_blocks;
#endif /* __BLOCKS__ */
} malloc_introspection_t_10_7;

typedef struct _malloc_zone_t_10_7 {
    /* Only zone implementors should depend on the layout of this structure;
	 Regular callers should use the access functions below */
    void	*reserved1;	/* RESERVED FOR CFAllocator DO NOT USE */
    void	*reserved2;	/* RESERVED FOR CFAllocator DO NOT USE */
    size_t 	(*size)(struct _malloc_zone_t *zone, const void *ptr); /* returns the size of a block or 0 if not in this zone; must be fast, especially for negative answers */
    void 	*(*malloc)(struct _malloc_zone_t *zone, size_t size);
    void 	*(*calloc)(struct _malloc_zone_t *zone, size_t num_items, size_t size); /* same as malloc, but block returned is set to zero */
    void 	*(*valloc)(struct _malloc_zone_t *zone, size_t size); /* same as malloc, but block returned is set to zero and is guaranteed to be page aligned */
    void 	(*free)(struct _malloc_zone_t *zone, void *ptr);
    void 	*(*realloc)(struct _malloc_zone_t *zone, void *ptr, size_t size);
    void 	(*destroy)(struct _malloc_zone_t *zone); /* zone is destroyed and all memory reclaimed */
    const char	*zone_name;

    /* Optional batch callbacks; these may be NULL */
    unsigned	(*batch_malloc)(struct _malloc_zone_t *zone, size_t size, void **results, unsigned num_requested); /* given a size, returns pointers capable of holding that size; returns the number of pointers allocated (maybe 0 or less than num_requested) */
    void	(*batch_free)(struct _malloc_zone_t *zone, void **to_be_freed, unsigned num_to_be_freed); /* frees all the pointers in to_be_freed; note that to_be_freed may be overwritten during the process */

    malloc_introspection_t_10_7	*introspect;
    unsigned	version;

    /* aligned memory allocation. The callback may be NULL. Present in version >= 5. */
    void *(*memalign)(struct _malloc_zone_t *zone, size_t alignment, size_t size);

    /* free a pointer known to be in zone and known to have the given size. The callback may be NULL. Present in version >= 6.*/
    void (*free_definite_size)(struct _malloc_zone_t *zone, void *ptr, size_t size);

    /* Empty out caches in the face of memory pressure. The callback may be NULL. Present in version >= 8. */
	size_t 	(*pressure_relief)(struct _malloc_zone_t *zone, size_t goal);
} malloc_zone_t_10_7;
