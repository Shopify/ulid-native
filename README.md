# ulid-native

This is a simple ruby extension wrapper for https://github.com/skeeto/ulid-c.

For simple single-threaded use, you can call `ULID.generate` to generate a ULID.

For multi-threaded use, or if you care about the generator mode, use:

```ruby
gen = ULID::Generator.new(format, flags = 0)
gen.generate
```

Format must be one of:

* `ULID::FORMAT_TEXT`: the default Base32 ULID format (26 bytes)
* `ULID::FORMAT_BINARY`: Raw binary ULID (15 bytes)
* `ULID::FORMAT_LEX62`: Lexicographically-ordered Base62 (21 bytes)

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

## Benchmarks

On my machine (a 2021 M1 Macbook Pro):

* `ULID::FORMAT_BINARY` generates in about 50ns;
* `ULID::FORMAT_TEXT` generates in about 100ns;
* `ULID::FORMAT_LEX62` generates in about 5000ns;
