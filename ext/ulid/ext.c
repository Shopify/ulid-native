#include "ext.h"
#include "ulid.h"
#include "ruby.h"
#include "ruby/encoding.h"
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
rb_encoding * encoding_binary;

VALUE fmt_text;
VALUE fmt_binary;
VALUE fmt_lex62;

struct ulid_generator * default_generator = NULL;
VALUE ulid_rb_generate_text_default(VALUE self);
VALUE ulid_rb_generate_binary_default(VALUE self);

VALUE ulid_generator_new(int argc, VALUE * argv, VALUE class)
{
  VALUE format = Qnil;
  VALUE flag = Qnil;
  rb_scan_args(argc, argv, "11", &format, &flag);
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

  gen->output_format = (void *) format;

  VALUE tdata = Data_Wrap_Struct(class, 0, free, gen);
  rb_obj_call_init(tdata, argc, argv);
  return tdata;
}

VALUE ulid_generator_initialize(int argc, VALUE * argv, VALUE self)
{
  // flag is first arg, or default zero
  VALUE format = Qnil;
  VALUE flag = Qnil;
  rb_scan_args(argc, argv, "11", &format, &flag);
  if (NIL_P(flag)) {
    flag = INT2NUM(0);
  }

  // changing these later will have no effect but we can at least preserve them
  // to make #inspect show something more useful.
  rb_iv_set(self, "@format", format);
  rb_iv_set(self, "@flags", flag);
  return self;
}

static VALUE
ulid_generator_generate(VALUE self)
{
  struct ulid_generator * gen;
  Data_Get_Struct(self, struct ulid_generator, gen);
  ulid_generate_binary(gen);
  VALUE fmt = (VALUE) gen->output_format;
  if (fmt == fmt_text) {
    char buf[27];
    ulid_encode(buf, gen->last);
    return rb_str_new(buf, 26);
  } else if (fmt == fmt_binary) {
    return rb_enc_str_new((const char *) gen->last, 16, encoding_binary);
  } else if (fmt == fmt_lex62) {
    rb_raise(rb_eRuntimeError, "not implemented yet");
  }
  rb_raise(rb_eRuntimeError, "unknown output format");
}

// decode. In: text(26); out: binary(16)
static VALUE
ulid_rb_decode(VALUE self, VALUE encoded)
{
  unsigned char out[16];
  if (RSTRING_LEN(encoded) != 26) {
    rb_raise(rb_eArgError, "text ULID must be 26 characters long");
  }
  ulid_decode(out, RSTRING_PTR(encoded));
  return rb_str_new((char *) out, 16);
}

// encode. In: binary(16); out: text(26)
static VALUE
ulid_rb_encode(VALUE self, VALUE decoded)
{
  char out[27];
  if (RSTRING_LEN(decoded) != 16) {
    rb_raise(rb_eArgError, "binary ULID must be 16 characters long");
  }
  ulid_encode(out, (const unsigned char *) RSTRING_PTR(decoded));
  return rb_str_new((char *) out, 26);
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
  encoding_binary = rb_enc_find("binary");

  rb_mULID = rb_define_module("ULID");
  rb_cULID_Generator = rb_define_class_under(rb_mULID, "Generator", rb_cObject);

  rb_define_singleton_method(rb_mULID, "encode", ulid_rb_encode, 1);
  rb_define_singleton_method(rb_mULID, "decode", ulid_rb_decode, 1);

  rb_define_singleton_method(rb_cULID_Generator, "new", ulid_generator_new, -1);
  rb_define_method(rb_cULID_Generator, "initialize", ulid_generator_initialize, -1);
  rb_define_method(rb_cULID_Generator, "generate", ulid_generator_generate, 0);

  rb_define_const(rb_mULID, "RELAXED", INT2NUM(ULID_RELAXED));
  rb_define_const(rb_mULID, "PARANOID", INT2NUM(ULID_PARANOID));
  rb_define_const(rb_mULID, "SECURE", INT2NUM(ULID_SECURE));

  fmt_text = rb_intern("text");
  fmt_binary = rb_intern("binary");
  fmt_lex62 = rb_intern("lex62");

  rb_define_const(rb_mULID, "FORMAT_TEXT", fmt_text);
  rb_define_const(rb_mULID, "FORMAT_BINARY", fmt_binary);
  rb_define_const(rb_mULID, "FORMAT_LEX62", fmt_lex62);

  default_generator = malloc(sizeof(struct ulid_generator));
  if (default_generator == NULL) {
    rb_raise(rb_eNoMemError, "could not allocate memory for ULID generator");
  }

  if (ulid_generator_init(default_generator, 0) != 0) {
    rb_raise(rb_eRuntimeError, "could not initialize ULID generator");
  }

  rb_define_singleton_method(rb_mULID, "generate_text", ulid_rb_generate_text_default, 0);
  rb_define_singleton_method(rb_mULID, "generate_binary", ulid_rb_generate_binary_default, 0);
}

VALUE
ulid_rb_generate_text_default(VALUE self)
{
  char buf[27];
  ulid_generate(default_generator, buf);
  return rb_str_new(buf, 26);
}

VALUE
ulid_rb_generate_binary_default(VALUE self)
{
  ulid_generate_binary(default_generator);
  return rb_enc_str_new((const char *) default_generator->last, 16, encoding_binary);
}
