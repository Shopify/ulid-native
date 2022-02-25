#include "ext.h"
#include "ulid.h"
#include "ruby.h"
#include "ruby/encoding.h"

#define LEX62_WIDTH 21

static VALUE rb_mULID;
static VALUE rb_cULID_Generator;
static rb_encoding * encoding_binary;

static VALUE fmt_text;
static VALUE fmt_binary;
static VALUE fmt_lex62;

struct ulid_generator * default_generator = NULL;
static VALUE ulid_rb_generate_text_default(VALUE self);
static VALUE ulid_rb_generate_binary_default(VALUE self);
static VALUE ulid_rb_generate_lex62_default(VALUE self);

int lex62_encode(char out[LEX62_WIDTH], const unsigned char in[16]);

static VALUE
ulid_generator_new(int argc, VALUE * argv, VALUE class)
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

static VALUE
ulid_generator_initialize(int argc, VALUE * argv, VALUE self)
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
    char buf[LEX62_WIDTH];
    if (0 != lex62_encode(buf, gen->last)) {
      rb_raise(rb_eRuntimeError, "could not encode ULID as lex62 due to value overflow");
    }
    VALUE what = rb_str_new(buf, LEX62_WIDTH);
    return what;
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
  rb_define_singleton_method(rb_mULID, "generate_lex62", ulid_rb_generate_lex62_default, 0);
}

static VALUE
ulid_rb_generate_text_default(VALUE self)
{
  char buf[27];
  ulid_generate(default_generator, buf);
  return rb_str_new(buf, 26);
}

static VALUE
ulid_rb_generate_binary_default(VALUE self)
{
  ulid_generate_binary(default_generator);
  return rb_enc_str_new((const char *) default_generator->last, 16, encoding_binary);
}

static VALUE
ulid_rb_generate_lex62_default(VALUE self)
{
  ulid_generate_binary(default_generator);
  char buf[LEX62_WIDTH];
  if (0 != lex62_encode(buf, default_generator->last)) {
    rb_raise(rb_eRuntimeError, "could not encode ULID as lex62 due to value overflow");
  }
  return rb_str_new(buf, LEX62_WIDTH);
}

static const char LEX62_CHARSET[] = "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz";

int
lex62_encode(char out[LEX62_WIDTH], const unsigned char in[16])
{
  __uint128_t val = 0;
  __uint128_t quot;
  int rem;
  int i;

  for (i = 0; i < 16; i++) {
    val <<= 8;
    val |= in[i];
  }

  for(i = 0; i < LEX62_WIDTH; i++) {
    // there's no div/lldiv for uint128_t that I could find. This way is faster
    // than doing both / and %.
    quot = val / 62;
    rem = (int)(val - (quot * 62)); 
    out[LEX62_WIDTH-i-1] = LEX62_CHARSET[rem];
    val = quot;
  }

  return (int) val; // should be 0 unless we overflowed
}
