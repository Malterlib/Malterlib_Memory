// Copyright © 2015 Hansoft AB
// Distributed under the MIT license, see license text in LICENSE.Malterlib

#pragma once

typedef struct _malloc_zone_t_known_version malloc_zone_t_known_version;

typedef struct malloc_introspection_t_known_version {
	kern_return_t (* MALLOC_INTROSPECT_FN_PTR(enumerator))(task_t task, void *, unsigned type_mask, vm_address_t zone_address, memory_reader_t reader, vm_range_recorder_t recorder); /* enumerates all the malloc pointers in use */
	size_t	(* MALLOC_INTROSPECT_FN_PTR(good_size))(malloc_zone_t_known_version *zone, size_t size);
	boolean_t 	(* MALLOC_INTROSPECT_FN_PTR(check))(malloc_zone_t_known_version *zone); /* Consistency checker */
	void	(* MALLOC_INTROSPECT_FN_PTR(print))(malloc_zone_t_known_version *zone, boolean_t verbose); /* Prints zone  */
	void	(* MALLOC_INTROSPECT_FN_PTR(log))(malloc_zone_t_known_version *zone, void * __unsafe_indexable address); /* Enables logging of activity */
	void	(* MALLOC_INTROSPECT_FN_PTR(force_lock))(malloc_zone_t_known_version *zone); /* Forces locking zone */
	void	(* MALLOC_INTROSPECT_FN_PTR(force_unlock))(malloc_zone_t_known_version *zone); /* Forces unlocking zone */
	void	(* MALLOC_INTROSPECT_FN_PTR(statistics))(malloc_zone_t_known_version *zone, malloc_statistics_t *stats); /* Fills statistics */
	boolean_t	(* MALLOC_INTROSPECT_FN_PTR(zone_locked))(malloc_zone_t_known_version *zone); /* Are any zone locks held */

	/* Discharge checking. Present in version >= 7. */
	boolean_t	(* MALLOC_INTROSPECT_FN_PTR(enable_discharge_checking))(malloc_zone_t_known_version *zone);
	void	(* MALLOC_INTROSPECT_FN_PTR(disable_discharge_checking))(malloc_zone_t_known_version *zone);
	void	(* MALLOC_INTROSPECT_FN_PTR(discharge))(malloc_zone_t_known_version *zone, void * __unsafe_indexable memory);
#ifdef __BLOCKS__
	void	(* MALLOC_INTROSPECT_FN_PTR(enumerate_discharged_pointers))(malloc_zone_t_known_version *zone, void (^report_discharged)(void *memory, void *info));
	#else
	void	*enumerate_unavailable_without_blocks;
#endif /* __BLOCKS__ */
	void	(* MALLOC_INTROSPECT_FN_PTR(reinit_lock))(malloc_zone_t_known_version *zone); /* Reinitialize zone locks, called only from atfork_child handler. Present in version >= 9. */
	void	(* MALLOC_INTROSPECT_FN_PTR(print_task))(task_t task, unsigned level, vm_address_t zone_address, memory_reader_t reader, print_task_printer_t printer); /* debug print for another process. Present in version >= 11. */
	void	(* MALLOC_INTROSPECT_FN_PTR(task_statistics))(task_t task, vm_address_t zone_address, memory_reader_t reader, malloc_statistics_t *stats); /* Present in version >= 12. */
	unsigned	zone_type; /* Identifies the zone type.  0 means unknown/undefined zone type.  Present in version >= 14. */
} malloc_introspection_t_known_version;

static_assert(sizeof(malloc_introspection_t_known_version) >= sizeof(malloc_introspection_t));

typedef struct _malloc_zone_t_known_version {
	void *reserved1;	/* RESERVED FOR CFAllocator DO NOT USE */
	void *reserved2;	/* RESERVED FOR CFAllocator DO NOT USE */

	/*
	 * Returns the size of a block or 0 if not in this zone; must be fast,
	 * especially for negative answers.
	 */
	size_t (* MALLOC_ZONE_FN_PTR(size))(struct _malloc_zone_t_known_version *zone,
			const void * __unsafe_indexable ptr);

	void * (* MALLOC_ZONE_FN_PTR(malloc))(
			struct _malloc_zone_t_known_version *zone, size_t size);

	/* Same as malloc, but block returned is set to zero */
	void * (* MALLOC_ZONE_FN_PTR(calloc))(
			struct _malloc_zone_t_known_version *zone, size_t num_items, size_t size);

	/* Same as malloc, but block returned is guaranteed to be page-aligned */
	void * (* MALLOC_ZONE_FN_PTR(valloc))(
			struct _malloc_zone_t_known_version *zone, size_t size);

	void (* MALLOC_ZONE_FN_PTR(free))(struct _malloc_zone_t_known_version *zone,
			void * __unsafe_indexable ptr);

	void * (* MALLOC_ZONE_FN_PTR(realloc))(
			struct _malloc_zone_t_known_version *zone, void * __unsafe_indexable ptr,
			size_t size);

	/* Zone is destroyed and all memory reclaimed */
	void (* MALLOC_ZONE_FN_PTR(destroy))(struct _malloc_zone_t_known_version *zone);

	const char * __null_terminated zone_name;

	/* Optional batch callbacks; these may be NULL */

	/*
	 * Given a size, returns pointers capable of holding that size; returns the
	 * number of pointers allocated (maybe 0 or less than num_requested)
	 */
	unsigned (* MALLOC_ZONE_FN_PTR(batch_malloc))(struct _malloc_zone_t_known_version *zone,
			size_t size,
			void * __unsafe_indexable * __counted_by(num_requested) results,
			unsigned num_requested);

	/*
	 * Frees all the pointers in to_be_freed; note that to_be_freed may be
	 * overwritten during the process
	 */
	void (* MALLOC_ZONE_FN_PTR(batch_free))(struct _malloc_zone_t_known_version *zone,
			void * __unsafe_indexable * __counted_by(num_to_be_freed) to_be_freed,
			unsigned num_to_be_freed);

	struct malloc_introspection_t_known_version * MALLOC_INTROSPECT_TBL_PTR(introspect);
	unsigned version;

	/* Aligned memory allocation. May be NULL.  Present in version >= 5. */
	void * (* MALLOC_ZONE_FN_PTR(memalign))(
			struct _malloc_zone_t_known_version *zone, size_t alignment, size_t size);

	/*
	 * Free a pointer known to be in zone and known to have the given size.
	 * May be NULL. Present in version >= 6.
	 */
	void (* MALLOC_ZONE_FN_PTR(free_definite_size))(struct _malloc_zone_t_known_version *zone,
			void * __sized_by(size) ptr, size_t size);

	/*
	 * Empty out caches in the face of memory pressure. May be NULL.
	 * Present in version >= 8.
	 */
	size_t (* MALLOC_ZONE_FN_PTR(pressure_relief))(struct _malloc_zone_t_known_version *zone,
			size_t goal);

	/*
	 * Checks whether an address might belong to the zone. May be NULL. Present
	 * in version >= 10.  False positives are allowed (e.g. the pointer was
	 * freed, or it's in zone space that has not yet been allocated. False
	 * negatives are not allowed.
	 */
	boolean_t (* MALLOC_ZONE_FN_PTR(claimed_address))(
			struct _malloc_zone_t_known_version *zone, void * __unsafe_indexable ptr);

	/*
	 * For libmalloc-internal zone 0 implementations only: try to free ptr,
	 * promising to call find_zone_and_free if it turns out not to belong to us.
	 * May be present in version >= 13.
	 */
	void (* MALLOC_ZONE_FN_PTR(try_free_default))(struct _malloc_zone_t_known_version *zone,
			void * __unsafe_indexable ptr);

	/*
	 * Memory allocation with an extensible binary flags option. Currently for
	 * libmalloc-internal zone implementations only - should be NULL otherwise.
	 * Added in version >= 15.
	 */
	void * (* MALLOC_ZONE_FN_PTR(malloc_with_options))(
			struct _malloc_zone_t_known_version *zone, size_t align, size_t size,
			uint64_t options);

	/*
	 * Typed Memory Operations versions of zone functions.  Present in
	 * version >= 16.
	 */

	void * (* MALLOC_ZONE_FN_PTR(malloc_type_malloc))(
			struct _malloc_zone_t_known_version *zone, size_t size, malloc_type_id_t type_id);

	void * (* MALLOC_ZONE_FN_PTR(malloc_type_calloc))(
			struct _malloc_zone_t_known_version *zone, size_t count, size_t size,
			malloc_type_id_t type_id);

	void * (* MALLOC_ZONE_FN_PTR(malloc_type_realloc))(
			struct _malloc_zone_t_known_version *zone, void * __unsafe_indexable ptr,
			size_t size, malloc_type_id_t type_id);

	void * (* MALLOC_ZONE_FN_PTR(malloc_type_memalign))(
			struct _malloc_zone_t_known_version *zone, size_t alignment, size_t size,
			malloc_type_id_t type_id);

	/* Must be NULL for non-libmalloc zone implementations */
	void * (* MALLOC_ZONE_FN_PTR(malloc_type_malloc_with_options))(
			struct _malloc_zone_t_known_version *zone, size_t align, size_t size, uint64_t options,
			malloc_type_id_t type_id);
} malloc_zone_t_known_version;

static_assert(sizeof(malloc_zone_t_known_version) >= sizeof(malloc_zone_t));

constexpr unsigned gc_KnownMallocZoneVersion = 16;
