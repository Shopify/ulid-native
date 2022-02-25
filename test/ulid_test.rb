# frozen_string_literal: true

require_relative('test_helper')

class ULIDTest < Minitest::Test
  TEXT = /^[0-9A-Z]{26}$/
  BINARY = /\A.{16}\z/m

  def test_generate
    assert_match(TEXT, ULID.generate)
    assert_match(BINARY, ULID.generate_binary)
  end

  def test_generator_flags
    check_generator(ULID::Generator.new)
    check_generator(ULID::Generator.new(ULID::RELAXED))
    check_generator(ULID::Generator.new(ULID::SECURE))
    check_generator(ULID::Generator.new(ULID::PARANOID))
    check_generator(ULID::Generator.new(ULID::RELAXED | ULID::SECURE))
    check_generator(ULID::Generator.new(ULID::PARANOID | ULID::SECURE))
  end

  private

  def check_generator(generator)
    assert_equal(Encoding::BINARY, generator.generate_binary.encoding)
    assert_match(TEXT, generator.generate)
    assert_match(BINARY, generator.generate_binary)
    assert_no_repeats { generator.generate }
    assert_no_repeats { generator.generate_binary }
  end

  def assert_no_repeats(&blk)
    els = 1000.times.map(&blk).uniq
    assert_equal(1000, els.size)
  end
end
