#include "ext.h"
#include "ulid.h"
#include "ruby.h"
#include <stdint.h>
#include <stdbool.h>
#include <sys/types.h>
#include <errno.h>
#include <fcntl.h>
#include <sys/stat.h>
#ifndef _WIN32
#include <sys/utsname.h>
#endif
#ifdef __GLIBC__
#include <gnu/libc-version.h>
#endif
#include <stdlib.h>

VALUE rb_mULID;
VALUE rb_cULID_Generator;

struct ulid_generator * default_generator = NULL;
VALUE ulid_rb_generate_default(VALUE self);

VALUE ulid_generator_new(int argc, VALUE * argv, VALUE class)
{
  // one optional argument, an integer defaulting to zero
  VALUE flag = Qnil;
  rb_scan_args(argc, argv, "01", &flag);
  if (NIL_P(flag)) {
    flag = INT2NUM(0);
  }

  struct ulid_generator * gen;
  gen = malloc(sizeof(struct ulid_generator));
  if (gen == NULL) {
    rb_raise(rb_eNoMemError, "could not allocate memory for ULID generator");
  }

  ulid_generator_init(gen, NUM2INT(flag));
  if (gen == NULL) {
    rb_raise(rb_eRuntimeError, "could not initialize ULID generator");
  }

  VALUE tdata = Data_Wrap_Struct(class, 0, free, gen);
  rb_obj_call_init(tdata, argc, argv);
  return tdata;
}

VALUE ulid_generator_initialize(int argc, VALUE * argv, VALUE self)
{
  // flag is first arg, or default zero
  VALUE flag = Qnil;
  rb_scan_args(argc, argv, "01", &flag);
  if (NIL_P(flag)) {
    flag = INT2NUM(0);
  }

  rb_iv_set(self, "@flags", flag);
  return self;
}

static VALUE
ulid_generator_generate(VALUE self)
{
  struct ulid_generator * gen;
  Data_Get_Struct(self, struct ulid_generator, gen);
  char buf[27];
  ulid_generate(gen, buf);
  return rb_str_new(buf, 26);
}

/*
 * Ruby C extensions are initialized by calling Init_<extname>.
 *
 * This sets up the module hierarchy and attaches functions as methods.
 *
 * We also populate some semi-static information about the current OS and so on.
 */
void
Init_ulid(void)
{
  rb_mULID = rb_define_module("ULID");
  rb_cULID_Generator = rb_define_class_under(rb_mULID, "Generator", rb_cObject);

  rb_define_singleton_method(rb_cULID_Generator, "new", ulid_generator_new, -1);
  rb_define_method(rb_cULID_Generator, "initialize", ulid_generator_initialize, -1);
  rb_define_method(rb_cULID_Generator, "generate", ulid_generator_generate, 0);

  rb_define_const(rb_mULID, "RELAXED", INT2NUM(ULID_RELAXED));
  rb_define_const(rb_mULID, "PARANOID", INT2NUM(ULID_PARANOID));
  rb_define_const(rb_mULID, "SECURE", INT2NUM(ULID_SECURE));

  default_generator = malloc(sizeof(struct ulid_generator));
  if (default_generator == NULL) {
    rb_raise(rb_eNoMemError, "could not allocate memory for ULID generator");
  }

  if (ulid_generator_init(default_generator, 0) != 0) {
    rb_raise(rb_eRuntimeError, "could not initialize ULID generator");
  }

  rb_define_singleton_method(rb_mULID, "generate", ulid_rb_generate_default, 0);
}

VALUE
ulid_rb_generate_default(VALUE self)
{
  char buf[27];
  ulid_generate(default_generator, buf);
  return rb_str_new(buf, 26);
}