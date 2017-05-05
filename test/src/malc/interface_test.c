#define MALC_GET_MIN_SEVERITY_FNAME malc_get_min_severity_test
#define MALC_LOG_FNAME              malc_log_test

#include <stdio.h>
#include <stdarg.h>
#include <string.h>

#include <bl/cmocka_pre.h>

#include <bl/base/integer.h>

#include <malc/malc.h>
#include <malc/alltypes.h>
#include <malc/stack_args.h>

#define FMT_STRING "value: {}"

/*----------------------------------------------------------------------------*/
typedef struct malc {
  malc_const_entry const* entry;
  uword                   size;
  alltypes                types;
}
malc;
/*----------------------------------------------------------------------------*/
MALC_EXPORT uword malc_get_min_severity_test (malc const*l )
{
  return malc_sev_note;
}
/*----------------------------------------------------------------------------*/
static inline u32 decode_comp_32 (malc_compressed_32 v)
{
  uword size = malc_compressed_get_size (v.format_nibble);
  uword neg  = malc_compressed_is_negative (v.format_nibble);
  u32 r = 0;
  for (uword i = 0; i < size; ++i) {
    r |= (v.v & (((u32) 0xff) << (i * 8)));
  }
  return (neg) ? ~r : r;
}
static inline u64 decode_comp_64 (malc_compressed_64 v)
{
  uword size = malc_compressed_get_size (v.format_nibble);
  uword neg  = malc_compressed_is_negative (v.format_nibble);
  u64 r = 0;
  for (uword i = 0; i < size; ++i) {
    r |= (v.v & (((u64) 0xff) << (i * 8)));
  }
  return (neg) ? ~r : r;
}
static inline void* decode_comp_ptr (malc_compressed_ptr v)
{
  uword size = malc_compressed_get_size (v.format_nibble);
  uword neg  = malc_compressed_is_negative (v.format_nibble);
  uword r = 0;
  for (uword i = 0; i < size; ++i) {
    r |= (v.v & (((uword) 0xff) << (i * 8)));
  }
  return (void*) ((neg) ? ~r : r);
}
/*----------------------------------------------------------------------------*/
MALC_EXPORT bl_err malc_log_test(
  struct malc* l, malc_const_entry const* e, uword size, ...
  )
{
  bl_err err = bl_ok;
  l->entry   = e;
  l->size    = size;

  va_list vargs;
  va_start (vargs, size);
  char const* partype = &e->info[1];

  while (*partype) {
    switch (*partype) {
    case malc_type_float: {
      l->types.vfloat = malc_get_va_arg (vargs, l->types.vfloat);
      break;
      }
    case malc_type_double: {
      l->types.vdouble = malc_get_va_arg (vargs, l->types.vdouble);
      break;
      }
    case malc_type_i8: {
      l->types.vi8 = malc_get_va_arg (vargs, l->types.vi8);
      break;
      }
    case malc_type_u8: {
      l->types.vu8 = malc_get_va_arg (vargs, l->types.vu8);
      break;
      }
    case malc_type_i16: {
      l->types.vi16 = malc_get_va_arg (vargs, l->types.vi16);
      break;
      }
    case malc_type_u16: {
      l->types.vu16 = malc_get_va_arg (vargs, l->types.vu16);
      break;
      }
#ifdef MALC_NO_BUILTIN_COMPRESSION
    case malc_type_i32: {
      l->types.vi32 = malc_get_va_arg (vargs, l->types.vi32);
      break;
      }
    case malc_type_u32: {
      l->types.vu32 = malc_get_va_arg (vargs, l->types.vu32);
      break;
      }
    case malc_type_i64: {
      l->types.vi64 = malc_get_va_arg (vargs, l->types.vi64);
      break;
      }
    case malc_type_u64: {
      l->types.vu64 = malc_get_va_arg (vargs, l->types.vu64);
      break;
      }
#else
    case malc_type_i32: {
      malc_compressed_32 v;
      v = malc_get_va_arg (vargs, v);
      l->types.vi32 = (i32) decode_comp_32 (v);
      break;
      }
    case malc_type_u32: {
      malc_compressed_32 v;
      v = malc_get_va_arg (vargs, v);
      l->types.vu32 = (u32) decode_comp_32 (v);
      break;
      }
    case malc_type_i64: {
      malc_compressed_64 v;
      v = malc_get_va_arg (vargs, v);
      l->types.vi64 = (i64) decode_comp_64 (v);
      break;
      }
    case malc_type_u64: {
      malc_compressed_64 v;
      v = malc_get_va_arg (vargs, v);
      l->types.vu64 = (u64) decode_comp_64 (v);
      break;
      }
#endif
#ifdef MALC_NO_PTR_COMPRESSION
    case malc_type_ptr: {
      l->types.vptr = malc_get_va_arg (vargs, l->types.vptr);
      break;
      }
    case malc_type_lit: {
      l->types.vlit = malc_get_va_arg (vargs, l->types.vlit);
      break;
      }
    case malc_type_strref: {
      l->types.vstrref = malc_get_va_arg (vargs, l->types.vstrref);
      break;
      }
    case malc_type_memref: {
      l->types.vmemref = malc_get_va_arg (vargs, l->types.vmemref);
      break;
      }
    case malc_type_refdtor: {
      l->types.vrefdtor = malc_get_va_arg (vargs, l->types.vrefdtor);
      break;
      }
#else
    case malc_type_ptr: {
      malc_compressed_ptr v;
      v = malc_get_va_arg (vargs, v);
      l->types.vptr = decode_comp_ptr (v);
      break;
      }
    case malc_type_lit: {
      malc_compressed_ptr v;
      v = malc_get_va_arg (vargs, v);
      l->types.vlit.lit = (char const*) decode_comp_ptr (v);
      break;
      }
    case malc_type_strref: {
      malc_compressed_ref v;
      v = malc_get_va_arg (vargs, v);
      l->types.vstrref.str = (char const*) decode_comp_ptr (v.ref);
      l->types.vstrref.len = v.size;
      break;
      }
    case malc_type_memref: {
      malc_compressed_ref v;
      v = malc_get_va_arg (vargs, v);
      l->types.vmemref.mem  = decode_comp_ptr (v.ref);
      l->types.vmemref.size = v.size;
      break;
      }
    case malc_type_refdtor: {
      malc_compressed_refdtor v;
      v = malc_get_va_arg (vargs, v);
      l->types.vrefdtor.func    = (malc_refdtor_fn) decode_comp_ptr (v.func);
      l->types.vrefdtor.context = decode_comp_ptr (v.context);
      break;
      }
#endif
    case malc_type_strcp: {
      l->types.vstrcp = malc_get_va_arg (vargs, l->types.vstrcp);
      break;
      }
    case malc_type_memcp: {
      l->types.vmemcp = malc_get_va_arg (vargs, l->types.vmemcp);
      break;
      }
    default: {
      err = bl_invalid;
      goto end_process_loop;
      }
    }
    ++partype;
  }
end_process_loop:
  va_end (vargs);
  return err;
}
/*----------------------------------------------------------------------------*/
static void interface_test_float (void **state)
{
  malc m;
  memset (&m, 0, sizeof m);
  bl_err err;
  float v = 12345.12345f;
  malc_error_i (err, &m, FMT_STRING, v);
  assert_int_equal (err, bl_ok);
  assert_true (v == m.types.vfloat);
  assert_true (m.size == sizeof (v));
  assert_true (m.entry->compressed_count == 0);
}
/*----------------------------------------------------------------------------*/
static void interface_test_double (void **state)
{
  malc m;
  memset (&m, 0, sizeof m);
  bl_err err;
  double v = 123451231234.12341231235;
  malc_error_i (err, &m, FMT_STRING, v);
  assert_int_equal (err, bl_ok);
  assert_true (v == m.types.vdouble);
  assert_true (m.size == sizeof (v));
  assert_true (m.entry->compressed_count == 0);
}
/*----------------------------------------------------------------------------*/
static void interface_test_i8 (void **state)
{
  malc m;
  memset (&m, 0, sizeof m);
  bl_err err;
  i8 v = -92;
  malc_error_i (err, &m, FMT_STRING, v);
  assert_int_equal (err, bl_ok);
  assert_true (v == m.types.vi8);
  assert_true (m.size == sizeof (v));
  assert_true (m.entry->compressed_count == 0);
}
/*----------------------------------------------------------------------------*/
static void interface_test_i16 (void **state)
{
  malc m;
  memset (&m, 0, sizeof m);
  bl_err err;
  i16 v = -92 * 256;
  malc_error_i (err, &m, FMT_STRING, v);
  assert_int_equal (err, bl_ok);
  assert_true (v == m.types.vi16);
  assert_true (m.size == sizeof (v));
  assert_true (m.entry->compressed_count == 0);
}
/*----------------------------------------------------------------------------*/
static void interface_test_i32 (void **state)
{
  malc m;
  memset (&m, 0, sizeof m);
  bl_err err;
  i32 v = -92 * 256 * 256 * 256;
  malc_error_i (err, &m, FMT_STRING, v);
  assert_int_equal (err, bl_ok);
  assert_true (v == m.types.vi32);
  assert_true (m.size == 4);
  v = -92;
  malc_error_i (err, &m, FMT_STRING, v);
  assert_int_equal (err, bl_ok);
  assert_true (v == m.types.vi32);
#ifdef MALC_NO_BUILTIN_COMPRESSION
  assert_true (m.size == 4);
  assert_true (m.entry->compressed_count == 0);
#else
  assert_true (m.size == 1);
  assert_true (m.entry->compressed_count == 1);
#endif
}
/*----------------------------------------------------------------------------*/
static void interface_test_i64 (void **state)
{
  malc m;
  memset (&m, 0, sizeof m);
  bl_err err;
  i64 v = -92 * ((u64) 1 << 58);
  malc_error_i (err, &m, FMT_STRING, v);
  assert_int_equal (err, bl_ok);
  assert_true (v == m.types.vi64);
  assert_true (m.size == 8);
  v = -92;
  malc_error_i (err, &m, FMT_STRING, v);
  assert_int_equal (err, bl_ok);
  assert_true (v == m.types.vi64);
#ifdef MALC_NO_BUILTIN_COMPRESSION
  assert_true (m.size == 8);
  assert_true (m.entry->compressed_count == 0);
#else
  assert_true (m.size == 1);
  assert_true (m.entry->compressed_count == 1);
#endif
}
/*----------------------------------------------------------------------------*/
static void interface_test_u8 (void **state)
{
  malc m;
  memset (&m, 0, sizeof m);
  bl_err err;
  u8 v = 92;
  malc_error_i (err, &m, FMT_STRING, v);
  assert_int_equal (err, bl_ok);
  assert_true (v == m.types.vu8);
  assert_true (m.size == sizeof (v));
  assert_true (m.entry->compressed_count == 0);
}
/*----------------------------------------------------------------------------*/
static void interface_test_u16 (void **state)
{
  malc m;
  memset (&m, 0, sizeof m);
  bl_err err;
  u16 v = 92 * 256;
  malc_error_i (err, &m, FMT_STRING, v);
  assert_int_equal (err, bl_ok);
  assert_true (v == m.types.vu16);
  assert_true (m.size == sizeof (v));
  assert_true (m.entry->compressed_count == 0);
}
/*----------------------------------------------------------------------------*/
static void interface_test_u32 (void **state)
{
  malc m;
  memset (&m, 0, sizeof m);
  bl_err err;
  u32 v = 92 * 256 * 256 * 256;
  malc_error_i (err, &m, FMT_STRING, v);
  assert_int_equal (err, bl_ok);
  assert_true (v == m.types.vu32);
  assert_true (m.size == 4);
  v = 92;
  malc_error_i (err, &m, FMT_STRING, v);
  assert_int_equal (err, bl_ok);
  assert_true (v == m.types.vu32);
#ifdef MALC_NO_BUILTIN_COMPRESSION
  assert_true (m.size == 4);
  assert_true (m.entry->compressed_count == 0);
#else
  assert_true (m.size == 1);
  assert_true (m.entry->compressed_count == 1);
#endif
}
/*----------------------------------------------------------------------------*/
static void interface_test_u64 (void **state)
{
  malc m;
  memset (&m, 0, sizeof m);
  bl_err err;
  u64 v = 92 * ((u64) 1 << 58);;
  malc_error_i (err, &m, FMT_STRING, v);
  assert_int_equal (err, bl_ok);
  assert_true (v == m.types.vu64);
  assert_true (m.size == 8);
  v = 92;
  malc_error_i (err, &m, FMT_STRING, v);
  assert_int_equal (err, bl_ok);
  assert_true (v == m.types.vu64);
#ifdef MALC_NO_BUILTIN_COMPRESSION
  assert_true (m.size == 8);
  assert_true (m.entry->compressed_count == 0);
#else
  assert_true (m.size == 1);
  assert_true (m.entry->compressed_count == 1);
#endif
}
/*----------------------------------------------------------------------------*/
static void interface_test_ptr (void **state)
{
  malc m;
  memset (&m, 0, sizeof m);
  bl_err err;
  void* v = (void*) 0xaa00;
  malc_error_i (err, &m, FMT_STRING, v);
  assert_int_equal (err, bl_ok);
  assert_true (v == m.types.vptr);
#ifdef MALC_NO_PTR_COMPRESSION
  assert_true (m.size == sizeof (v));
  assert_true (m.entry->compressed_count == 0);
#else
  assert_true (m.size == 2);
  assert_true (m.entry->compressed_count == 1);
#endif
}
/*----------------------------------------------------------------------------*/
static void interface_test_lit (void **state)
{
  malc m;
  memset (&m, 0, sizeof m);
  bl_err err;
  malc_lit v = {(char const*) 0xaa00 };
  malc_error_i (err, &m, FMT_STRING, v);
  assert_int_equal (err, bl_ok);
  assert_true (v.lit == m.types.vlit.lit);
#ifdef MALC_NO_PTR_COMPRESSION
  assert_true (m.size == sizeof (v));
  assert_true (m.entry->compressed_count == 0);
#else
  assert_true (m.size == 2);
  assert_true (m.entry->compressed_count == 1);
#endif
}
/*----------------------------------------------------------------------------*/
static void interface_test_strcp (void **state)
{
  malc m;
  memset (&m, 0, sizeof m);
  bl_err err;
  malc_strcp v = {(char const*) 0xaa00aa00, 12345 };
  malc_error_i (err, &m, FMT_STRING, v);
  assert_int_equal (err, bl_ok);
  assert_true (v.str == m.types.vstrcp.str);
  assert_true (v.len == m.types.vstrcp.len);
  assert_true (m.size == sizeof (u16) + m.types.vstrcp.len);
  assert_true (m.entry->compressed_count == 0);
}
/*----------------------------------------------------------------------------*/
static void interface_test_strref (void **state)
{
  malc m;
  memset (&m, 0, sizeof m);
  bl_err err;
  malc_strref  v = {(char const*)     0xaa00aa00, 12345 };
  malc_refdtor d = {(malc_refdtor_fn) 0xaa00aa00, (void*) 0x12125656 };
  malc_error_i (err, &m, FMT_STRING, v, d);
  assert_int_equal (err, bl_ok);
  assert_true (v.str == m.types.vstrref.str);
  assert_true (v.len == m.types.vstrref.len);
#ifdef MALC_NO_PTR_COMPRESSION
  assert_true(
    m.size == sizeof d.func + sizeof d.context + sizeof v.str + sizeof v.len
    );
  assert_true (m.entry->compressed_count == 0);
#else
  assert_true (m.size == 4 + sizeof (u16) + 4 + 4);
  assert_true (m.entry->compressed_count == 3);
#endif
}
/*----------------------------------------------------------------------------*/
static void interface_test_memcp (void **state)
{
  malc m;
  memset (&m, 0, sizeof m);
  bl_err err;
  malc_memcp v = {(u8 const*) 0xaa00aa00, 12345 };
  malc_error_i (err, &m, FMT_STRING, v);
  assert_int_equal (err, bl_ok);
  assert_true (v.mem == m.types.vmemcp.mem);
  assert_true (v.size == m.types.vmemcp.size);
  assert_true (m.size == sizeof (u16) + m.types.vmemcp.size);
  assert_true (m.entry->compressed_count == 0);
}
/*----------------------------------------------------------------------------*/
static void interface_test_memref (void **state)
{
  malc m;
  memset (&m, 0, sizeof m);
  bl_err err;
  malc_memref  v = {(u8 const*)       0xaa00aa00, 12345 };
  malc_refdtor d = {(malc_refdtor_fn) 0xaa00aa00, (void*) 0x12125656 };
  malc_error_i (err, &m, FMT_STRING, v, d);
  assert_int_equal (err, bl_ok);
  assert_true (v.mem == m.types.vmemref.mem);
  assert_true (v.size == m.types.vmemref.size);
#ifdef MALC_NO_PTR_COMPRESSION
  assert_true(
    m.size == (sizeof d.func + sizeof d.context + sizeof v.mem + sizeof v.size)
    );
  assert_true (m.entry->compressed_count == 0);
#else
  assert_true (m.size == 4 + sizeof (u16) + 4 + 4);
  assert_true (m.entry->compressed_count == 3);
#endif
}
/*----------------------------------------------------------------------------*/
static void interface_test_all (void **state)
{
  malc m;
  alltypes all;
  memset (&m, 0, sizeof m);
  memset (&all, 0, sizeof all);
  char expected_info_str[] = {
    malc_sev_error,
    malc_type_u8,
    malc_type_i8,
    malc_type_u16,
    malc_type_i16,
    malc_type_u32,
    malc_type_i32,
    malc_type_u64,
    malc_type_i64,
    malc_type_float,
    malc_type_double,
    malc_type_ptr,
    malc_type_strcp,
    malc_type_lit,
    malc_type_memcp,
    malc_type_strref,
    malc_type_memref,
    malc_type_refdtor,
    0
  };

  all.vu8              = 2;
  all.vi8              = 5;
  all.vu16             = 23422;
  all.vi16             = -22222;
  all.vu32             = 23459999;
  all.vi32             = -234243442;
  all.vu64             = 3222222222222222222;
  all.vi64             = -5666666666566666666;
  all.vfloat           = 195953.2342f;
  all.vdouble          = 1231231123123123.234234444;
  all.vptr             = (void*) 0xaa40aa00;
  all.vstrcp.str       = (char const*) 0x123123;
  all.vstrcp.len       = 12;
  all.vlit.lit         = (char const*) 0x16783123;
  all.vmemcp.mem       = (u8 const*) 0xaa55aa55;
  all.vmemcp.size      = 2345;
  all.vstrref.str      = (char const*) 0xaa00aa11;
  all.vstrref.len      = 88;
  all.vmemref.mem      = (u8 const*) 0xaa00aa22;
  all.vmemref.size     = 169;
  all.vrefdtor.func    = (malc_refdtor_fn) 0xaa44aa00;
  all.vrefdtor.context = (void*) 0x12125656;

  bl_err err;
  malc_error_i(
    err,
    &m,
    "{} {} {} {} {} {} {} {} {} {} {} {} {} {}",
    all.vu8,
    all.vi8,
    all.vu16,
    all.vi16,
    all.vu32,
    all.vi32,
    all.vu64,
    all.vi64,
    all.vfloat,
    all.vdouble,
    all.vptr,
    all.vstrcp,
    all.vlit,
    all.vmemcp,
    all.vstrref,
    all.vmemref,
    all.vrefdtor
    );

  assert_int_equal (err, bl_ok);
  assert_memory_equal (&all, &m.types, sizeof all);
  assert_string_equal (m.entry->info, expected_info_str);
}
/*----------------------------------------------------------------------------*/
/* this is to test the compiler's preprocessor (verify that it compiles) */
static void interface_test_casting (void **state)
{
  malc m;
  memset (&m, 0, sizeof m);
  bl_err err;
  i16 v = -92 * 256;
  malc_error_i (err, &m, FMT_STRING, (u16) v);
  assert_int_equal (err, bl_ok);
  assert_true ((u16) v == m.types.vu16);
  assert_true (m.size == sizeof (v));
  assert_true (m.entry->compressed_count == 0);
}
/*----------------------------------------------------------------------------*/
static u8 a_function (i16 v) { return (u8) v; }
/*----------------------------------------------------------------------------*/
/* this is to test the compiler's preprocessor (verify that  it compiles) */
static void interface_test_func_call_with_casting (void **state)
{
  malc m;
  memset (&m, 0, sizeof m);
  bl_err err;
  i16 v = -92 * 256;
  malc_error_i (err, &m, FMT_STRING, (u16) a_function (v));
  assert_int_equal (err, bl_ok);
  assert_true ((u16) a_function (v) == m.types.vu16);
  assert_true (m.size == sizeof (v));
  assert_true (m.entry->compressed_count == 0);
}
/*----------------------------------------------------------------------------*/
/* this is to test the compiler's preprocessor (verify that  it compiles) */
static void interface_test_ternary (void **state)
{
  malc m;
  memset (&m, 0, sizeof m);
  bl_err err;
  uword choice = (((uword) &err) > 4) & 1;
  malc_error_i (err, &m, FMT_STRING, (u16) (choice ? (u16) 1 : (u16) 2));
  assert_int_equal (err, bl_ok);
  assert_true ((u16) (choice ? 1 : 2) == m.types.vu16);
  assert_true (m.entry->compressed_count == 0);
}
/*----------------------------------------------------------------------------*/
static const struct CMUnitTest tests[] = {
  cmocka_unit_test (interface_test_float),
  cmocka_unit_test (interface_test_double),
  cmocka_unit_test (interface_test_i8),
  cmocka_unit_test (interface_test_i16),
  cmocka_unit_test (interface_test_i32),
  cmocka_unit_test (interface_test_i64),
  cmocka_unit_test (interface_test_u8),
  cmocka_unit_test (interface_test_u16),
  cmocka_unit_test (interface_test_u32),
  cmocka_unit_test (interface_test_u64),
  cmocka_unit_test (interface_test_ptr),
  cmocka_unit_test (interface_test_lit),
  cmocka_unit_test (interface_test_strcp),
  cmocka_unit_test (interface_test_strref),
  cmocka_unit_test (interface_test_memcp),
  cmocka_unit_test (interface_test_memref),
  cmocka_unit_test (interface_test_all),
  cmocka_unit_test (interface_test_casting),
  cmocka_unit_test (interface_test_func_call_with_casting),
  cmocka_unit_test (interface_test_ternary),
};
/*----------------------------------------------------------------------------*/
int interface_tests (void)
{
  return cmocka_run_group_tests (tests, nullptr, nullptr);
}
/*----------------------------------------------------------------------------*/
