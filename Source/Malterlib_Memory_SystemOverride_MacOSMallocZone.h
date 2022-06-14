// Copyright © 2015 Hansoft AB
// Distributed under the MIT license, see license text in LICENSE.Malterlib

#pragma once

typedef struct malloc_introspection_t_10_7 {
	kern_return_t (* MALLOC_INTROSPECT_FN_PTR(enumerator))(task_t task, void *, unsigned type_mask, vm_address_t zone_address, memory_reader_t reader, vm_range_recorder_t recorder); /* enumerates all the malloc pointers in use */
	size_t	(* MALLOC_INTROSPECT_FN_PTR(good_size))(malloc_zone_t *zone, size_t size);
	boolean_t 	(* MALLOC_INTROSPECT_FN_PTR(check))(malloc_zone_t *zone); /* Consistency checker */
	void 	(* MALLOC_INTROSPECT_FN_PTR(print))(malloc_zone_t *zone, boolean_t verbose); /* Prints zone  */
	void	(* MALLOC_INTROSPECT_FN_PTR(log))(malloc_zone_t *zone, void *address); /* Enables logging of activity */
	void	(* MALLOC_INTROSPECT_FN_PTR(force_lock))(malloc_zone_t *zone); /* Forces locking zone */
	void	(* MALLOC_INTROSPECT_FN_PTR(force_unlock))(malloc_zone_t *zone); /* Forces unlocking zone */
	void	(* MALLOC_INTROSPECT_FN_PTR(statistics))(malloc_zone_t *zone, malloc_statistics_t *stats); /* Fills statistics */
	boolean_t   (* MALLOC_INTROSPECT_FN_PTR(zone_locked))(malloc_zone_t *zone); /* Are any zone locks held */

	/* Discharge checking. Present in version >= 7. */
	boolean_t	(* MALLOC_INTROSPECT_FN_PTR(enable_discharge_checking))(malloc_zone_t *zone);
	void	(* MALLOC_INTROSPECT_FN_PTR(disable_discharge_checking))(malloc_zone_t *zone);
	void	(* MALLOC_INTROSPECT_FN_PTR(discharge))(malloc_zone_t *zone, void *memory);
#ifdef __BLOCKS__
	void     (* MALLOC_INTROSPECT_FN_PTR(enumerate_discharged_pointers))(malloc_zone_t *zone, void (^report_discharged)(void *memory, void *info));
	#else
	void	*enumerate_unavailable_without_blocks;
#endif /* __BLOCKS__ */
} malloc_introspection_t_10_7;

typedef struct _malloc_zone_t_10_7 {
	/* Only zone implementors should depend on the layout of this structure;
	 Regular callers should use the access functions below */
	void	*reserved1;	/* RESERVED FOR CFAllocator DO NOT USE */
	void	*reserved2;	/* RESERVED FOR CFAllocator DO NOT USE */
	size_t 	(* MALLOC_ZONE_FN_PTR(size))(struct _malloc_zone_t *zone, const void *ptr); /* returns the size of a block or 0 if not in this zone; must be fast, especially for negative answers */
	void 	*(* MALLOC_ZONE_FN_PTR(malloc))(struct _malloc_zone_t *zone, size_t size);
	void 	*(* MALLOC_ZONE_FN_PTR(calloc))(struct _malloc_zone_t *zone, size_t num_items, size_t size); /* same as malloc, but block returned is set to zero */
	void 	*(* MALLOC_ZONE_FN_PTR(valloc))(struct _malloc_zone_t *zone, size_t size); /* same as malloc, but block returned is set to zero and is guaranteed to be page aligned */
	void 	(* MALLOC_ZONE_FN_PTR(free))(struct _malloc_zone_t *zone, void *ptr);
	void 	*(* MALLOC_ZONE_FN_PTR(realloc))(struct _malloc_zone_t *zone, void *ptr, size_t size);
	void 	(* MALLOC_ZONE_FN_PTR(destroy))(struct _malloc_zone_t *zone); /* zone is destroyed and all memory reclaimed */
	const char	*zone_name;

	/* Optional batch callbacks; these may be NULL */
	unsigned	(* MALLOC_ZONE_FN_PTR(batch_malloc))(struct _malloc_zone_t *zone, size_t size, void **results, unsigned num_requested); /* given a size, returns pointers capable of holding that size; returns the number of pointers allocated (maybe 0 or less than num_requested) */
	void	(* MALLOC_ZONE_FN_PTR(batch_free))(struct _malloc_zone_t *zone, void **to_be_freed, unsigned num_to_be_freed); /* frees all the pointers in to_be_freed; note that to_be_freed may be overwritten during the process */

	struct malloc_introspection_t_10_7	* MALLOC_INTROSPECT_TBL_PTR(introspect);
	unsigned	version;

	/* aligned memory allocation. The callback may be NULL. Present in version >= 5. */
	void *(* MALLOC_ZONE_FN_PTR(memalign))(struct _malloc_zone_t *zone, size_t alignment, size_t size);

	/* free a pointer known to be in zone and known to have the given size. The callback may be NULL. Present in version >= 6.*/
	void (* MALLOC_ZONE_FN_PTR(free_definite_size))(struct _malloc_zone_t *zone, void *ptr, size_t size);

	/* Empty out caches in the face of memory pressure. The callback may be NULL. Present in version >= 8. */
	size_t 	(* MALLOC_ZONE_FN_PTR(pressure_relief))(struct _malloc_zone_t *zone, size_t goal);
} malloc_zone_t_10_7;
