# frozen_string_literal: true

require_relative('test_helper')

class ULIDTest < Minitest::Test
  TEXT = /^[0-9A-Z]{26}$/
  BINARY = /\A.{16}\z/m

  def test_generate
    assert_match(TEXT, ULID.generate_text)
    assert_match(BINARY, ULID.generate_binary)
  end

  def test_generator_flags
    [ULID::FORMAT_TEXT, ULID::FORMAT_BINARY].each do |fmt|
      check_generator(ULID::Generator.new(fmt))
      check_generator(ULID::Generator.new(fmt, ULID::RELAXED))
      check_generator(ULID::Generator.new(fmt, ULID::SECURE))
      check_generator(ULID::Generator.new(fmt, ULID::PARANOID))
      check_generator(ULID::Generator.new(fmt, ULID::RELAXED | ULID::SECURE))
      check_generator(ULID::Generator.new(fmt, ULID::PARANOID | ULID::SECURE))
    end
  end

  def check_encoding
    gen = ULID::Generator.new(ULID::FORMAT_BINARY)
    assert_equal(Encoding::BINARY, gen.generate.encoding)
  end

  private

  def check_generator(generator)
    case generator.instance_variable_get(:@format)
    when ULID::FORMAT_TEXT
      assert_match(TEXT, generator.generate)
    when ULID::FORMAT_BINARY
      assert_match(BINARY, generator.generate)
    else
      raise("Unknown format: #{generator.instance_variable_get(:@format)}")
    end
    assert_no_repeats { generator.generate }
  end

  def assert_no_repeats(&blk)
    els = 1000.times.map(&blk).uniq
    assert_equal(1000, els.size)
  end
end
