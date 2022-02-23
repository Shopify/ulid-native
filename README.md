# ulid-native

This is a simple ruby extension wrapper for https://github.com/skeeto/ulid-c.

For simple single-threaded use, you can call `ULID.generate` to generate a ULID.

For multi-threaded use, or if you care about the generator mode, use:

```ruby
gen = ULID::Generator.new(flags)
gen.generate
```

Flags can be zero or more (i.e. `a | b`) of:

* `ULID::RELAXED`: allows ULIDs generated within the same millisecond can be
  non-monotonic, e.g. the random section is generated fresh each time.
* `ULID::PARANOID`: Causes the generator to clear the highest bit
  of the random field, which guarantees that overflow cannot occur.
  Normally the chance of overflow is non-zero, but negligible. This
  makes it zero. It doesn't make sense to use this flag in conjunction
  with `ULID::RELAXED`.
* `ULID::SECURE`: doesn't fall back on generated entropy for initialization if
  system entropy could not be gathered. An error is raised instead.

## Binary and Text Encoding

`generate` returns a text-formatted ULID, but `generate_binary` also exists on
both the `Generator` class and the `ULID` module, and returns a binary
representation. `ULID.encode` and `ULID.decode` exist to convert between these
formats.

On my machine (a 2021 M1 Macbook Pro), generating the `generate` with default
flags takes 100ns, and `generate_binary` takes 50ns.
