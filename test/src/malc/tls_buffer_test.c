#include <string.h>

#include <bl/cmocka_pre.h>

#include <malc/tls_buffer.h>
#include <malc/test_allocator.h>

#include <bl/base/allocator.h>
#include <bl/base/integer.h>
#include <bl/base/utility.h>
#include <bl/base/static_integer_math.h>

static const uword tls_buff_slots     = 4;
static const uword tls_buff_slot_size = 32;
/*----------------------------------------------------------------------------*/
typedef struct tls_context {
  alloc_data  alloc;
  u8          buffer[sizeof (tls_buffer) + 1024];
  tls_buffer* t;
}
tls_context;
/*----------------------------------------------------------------------------*/
static int tls_test_setup (void **state)
{
  static tls_context c;
  memset (c.buffer, 0, sizeof c.buffer);
  c.t     = nullptr;
  c.alloc = alloc_data_init (c.buffer);
  *state  = (void*) &c;
  return 0;
}
/*----------------------------------------------------------------------------*/
static int tls_test_init_setup (void **state)
{
  tls_test_setup (state);
  tls_context* c = (tls_context*) *state;
  bl_err err = tls_buffer_init(
    &c->t, tls_buff_slot_size, tls_buff_slots, &c->alloc.alloc, nullptr, nullptr
    );
  assert_true (!err);
  return 0;
}
/*----------------------------------------------------------------------------*/
static void tls_init_test (void **state)
{
  tls_buffer* t;
  tls_context* c = (tls_context*) *state;
  bl_err err = tls_buffer_init(
    &t, tls_buff_slot_size, tls_buff_slots, &c->alloc.alloc, nullptr, nullptr
    );
  assert_int_equal (err, bl_ok);
  assert_true (t->destructor_fn == nullptr);
  assert_true (t->destructor_context == nullptr);
  assert_true (t->mem >= ((u8*) t) + sizeof (*t));
  assert_true (is_multiple ((uword) t->mem, tls_buff_slot_size));
  assert_true (t->slot_count == tls_buff_slots);
  assert_true (t->slot_size == tls_buff_slot_size);
}
/*----------------------------------------------------------------------------*/
static void tls_single_alloc_test (void **state)
{
  tls_context* c = (tls_context*) *state;
  u8* mem;
  bl_err err = tls_buffer_alloc (c->t, &mem, 1);
  assert_int_equal (err, bl_ok);
  assert_ptr_equal (mem, c->t->mem);
}
/*----------------------------------------------------------------------------*/
static void tls_double_alloc_test (void **state)
{
  tls_context* c = (tls_context*) *state;
  u8* mem;
  bl_err err = tls_buffer_alloc (c->t, &mem, 1);
  assert_int_equal (err, bl_ok);
  assert_ptr_equal (mem, c->t->mem);
  *((uword*) mem) = 1; /*0 (null) on the first signal marks a free slot*/
  err = tls_buffer_alloc (c->t, &mem, 1);
  assert_int_equal (err, bl_ok);
  assert_ptr_equal (mem, c->t->mem + tls_buff_slot_size);
}
/*----------------------------------------------------------------------------*/
static void tls_single_alloc_too_big_test (void **state)
{
  tls_context* c = (tls_context*) *state;
  u8* mem;
  bl_err err = tls_buffer_alloc (c->t, &mem, tls_buff_slots + 1);
  assert_int_equal (err, bl_would_overflow);
}
/*----------------------------------------------------------------------------*/
static void tls_double_alloc_too_big_test (void **state)
{
  tls_context* c = (tls_context*) *state;
  u8* mem;
  bl_err err = tls_buffer_alloc (c->t, &mem, tls_buff_slots);
  assert_int_equal (err, bl_ok);
  assert_ptr_equal (mem, c->t->mem);
  *((uword*) mem) = 1; /*0 (null) on the first signal marks a free slot*/
  err = tls_buffer_alloc (c->t, &mem, 1);
  assert_int_equal (err, bl_would_overflow);
}
/*----------------------------------------------------------------------------*/
static void tls_full_expand_test (void **state)
{
  tls_context* c = (tls_context*) *state;
  u8* mem;
  bl_err err = tls_buffer_alloc (c->t, &mem, 1);
  assert_int_equal (err, bl_ok);
  assert_ptr_equal (mem, c->t->mem);
  *((uword*) mem) = 1; /*0 (null) on the first signal marks a free slot*/
  u8* newmem;
  err = tls_buffer_expand (c->t, &newmem, mem, tls_buff_slots - 1);
  assert_int_equal (err, bl_ok);
  assert_ptr_equal (mem, c->t->mem);
}
/*----------------------------------------------------------------------------*/
static void tls_full_expand_too_big_test (void **state)
{
  tls_context* c = (tls_context*) *state;
  u8* mem;
  bl_err err = tls_buffer_alloc (c->t, &mem, 1);
  assert_int_equal (err, bl_ok);
  assert_ptr_equal (mem, c->t->mem);
  *((uword*) mem) = 1; /*0 (null) on the first signal marks a free slot*/
  u8* newmem;
  err = tls_buffer_expand (c->t, &newmem, mem, tls_buff_slots);
  assert_int_equal (err, bl_would_overflow);
}
/*----------------------------------------------------------------------------*/
static void tls_dealloc_test (void **state)
{
  tls_context* c = (tls_context*) *state;
  u8* mem;
  bl_err err = tls_buffer_alloc (c->t, &mem, tls_buff_slots);
  assert_int_equal (err, bl_ok);
  assert_ptr_equal (mem, c->t->mem);
  *((uword*) mem) = 1; /*0 (null) on the first signal marks a free slot*/
  tls_buffer_dealloc (mem, tls_buff_slots, tls_buff_slot_size);
  err = tls_buffer_alloc (c->t, &mem, tls_buff_slots);
  assert_int_equal (err, bl_ok);
  assert_ptr_equal (mem, c->t->mem);
}
/*----------------------------------------------------------------------------*/
static void tls_single_wrap_test (void **state)
{
  tls_context* c = (tls_context*) *state;
  u8* mem;
  bl_err err = tls_buffer_alloc (c->t, &mem, tls_buff_slots - 1);
  assert_int_equal (err, bl_ok);
  assert_ptr_equal (mem, c->t->mem);
  *((uword*) mem) = 1; /*0 (null) on the first signal marks a free slot*/
  u8* mem2;
  err = tls_buffer_alloc (c->t, &mem2, 1);
  assert_int_equal (err, bl_ok);
  assert_ptr_equal(
    mem2, c->t->mem + (tls_buff_slot_size * (tls_buff_slots - 1))
    );
  *((uword*) mem2) = 1; /*0 (null) on the first signal marks a free slot*/
  tls_buffer_dealloc (mem, tls_buff_slots, tls_buff_slot_size);

  err = tls_buffer_alloc (c->t, &mem, tls_buff_slots);
  assert_int_equal (err, bl_ok);
  assert_ptr_equal (mem, c->t->mem);
}
/*----------------------------------------------------------------------------*/
static void tls_multiple_wrap_test (void **state)
{
  tls_context* c = (tls_context*) *state;
  u8* mem;
  bl_err err = tls_buffer_alloc (c->t, &mem, tls_buff_slots - 1);
  assert_int_equal (err, bl_ok);
  assert_ptr_equal (mem, c->t->mem);
  *((uword*) mem) = 1; /*0 (null) on the first signal marks a free slot*/
  tls_buffer_dealloc (mem, tls_buff_slots - 1, tls_buff_slot_size);
  err = tls_buffer_alloc (c->t, &mem, tls_buff_slots - 1);
  assert_int_equal (err, bl_ok);
  assert_ptr_equal (mem, c->t->mem);
}
/*----------------------------------------------------------------------------*/
static const struct CMUnitTest tests[] = {
  cmocka_unit_test_setup (tls_init_test, tls_test_setup),
  cmocka_unit_test_setup (tls_single_alloc_test, tls_test_init_setup),
  cmocka_unit_test_setup (tls_double_alloc_test, tls_test_init_setup),
  cmocka_unit_test_setup (tls_single_alloc_too_big_test, tls_test_init_setup),
  cmocka_unit_test_setup (tls_double_alloc_too_big_test, tls_test_init_setup),
  cmocka_unit_test_setup (tls_full_expand_test, tls_test_init_setup),
  cmocka_unit_test_setup (tls_full_expand_too_big_test, tls_test_init_setup),
  cmocka_unit_test_setup (tls_dealloc_test, tls_test_init_setup),
  cmocka_unit_test_setup (tls_single_wrap_test, tls_test_init_setup),
  cmocka_unit_test_setup (tls_multiple_wrap_test, tls_test_init_setup),
};
/*----------------------------------------------------------------------------*/
int tls_buffer_tests (void)
{
  return cmocka_run_group_tests (tests, nullptr, nullptr);
}
/*----------------------------------------------------------------------------*/
