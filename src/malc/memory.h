#ifndef __MALC_ALLOCATOR__
#define __MALC_ALLOCATOR__

#include <bl/base/integer.h>
#include <bl/base/allocator.h>
#include <bl/base/error.h>
#include <bl/base/dynarray.h>
#include <bl/base/static_integer_math.h>

#include <malc/cfg.h>
#include <malc/tls_buffer.h>
#include <malc/bounded_buffer.h>

/*----------------------------------------------------------------------------*/
#define alloc_slot_size 32
#define alloc_slot_size_log2 (static_log2_ceil_u (alloc_slot_size))
/*----------------------------------------------------------------------------*/
enum alloc_tags {
  alloc_tag_free    = 0,
  alloc_tag_tls     = 1,
  alloc_tag_bounded = 2,
  alloc_tag_heap    = 3,
  alloc_tag_total   = 4,
};
typedef u8 alloc_tag;
#define alloc_tag_bits static_log2_ceil_u (alloc_tag_total)
#define alloc_tag_mask (u_lsb_set (alloc_tag_bits))
/*----------------------------------------------------------------------------*/
define_dynarray_types (mem_array, void*)
/*----------------------------------------------------------------------------*/
typedef struct memory {
  malc_alloc_cfg cfg;
  bl_tss         tss_key;
  mem_array      tss_list;
  boundedb       bb;
}
memory;
/*----------------------------------------------------------------------------*/
extern bl_err memory_init (memory* m, alloc_tbl const* alloc);
/*----------------------------------------------------------------------------*/
extern void memory_destroy (memory* m, alloc_tbl const* alloc);
/*----------------------------------------------------------------------------*/
extern bl_err memory_tls_init_unregistered(
  memory*          m,
  u32              bytes,
  alloc_tbl const* alloc,
  tls_destructor   thread_exit_destructor,
  void*            thread_exit_destructor_context,
  void**           tls_buffer_addr
  );
/*----------------------------------------------------------------------------*/
extern bl_err memory_bounded_buffer_init (memory* m, alloc_tbl const* alloc);
/*----------------------------------------------------------------------------*/
extern bl_err memory_alloc (memory* m, u8** mem, alloc_tag* tag, u32 slots);
/*----------------------------------------------------------------------------*/
extern void memory_dealloc (memory* m, u8* mem, alloc_tag tag, u32 slots);
/*----------------------------------------------------------------------------*/
extern bl_err memory_tls_register(
  memory* m, void* mem, alloc_tbl const* alloc
  );
/*----------------------------------------------------------------------------*/
extern bool memory_tls_destroy (memory* m, void* mem, alloc_tbl const* alloc);
/*----------------------------------------------------------------------------*/
extern void memory_tls_destroy_all (memory* m, alloc_tbl const* alloc);
/*----------------------------------------------------------------------------*/

#endif /* __MALC_ALLOCATOR__ */