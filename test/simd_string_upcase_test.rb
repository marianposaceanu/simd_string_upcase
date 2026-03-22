require_relative 'test_helper'

class SIMDStringUpcaseTest < Minitest::Test
  def test_string_upcase_matches_original_for_ascii_inputs
    inputs = [
      '',
      'a',
      'hello, world!',
      'abc123xyz',
      'a' * 15,
      'a' * 16,
      'a' * 17,
      'a' * 31,
      'a' * 32,
      'a' * 33,
      'a' * 63,
      'a' * 64,
      'a' * 65,
      ('abc123xyz! ' * 20)
    ]

    inputs.each do |input|
      assert_equal input.original_upcase, input.upcase
    end
  end

  def test_upcase_does_not_mutate_the_original_string
    input = 'mixed Case 123!'

    result = input.upcase

    assert_equal 'mixed Case 123!', input
    assert_equal 'MIXED CASE 123!', result
  end

  def test_upcase_preserves_encoding_for_ascii_strings
    input = 'plain ascii'.encode(Encoding::UTF_8)

    assert_equal input.encoding, input.upcase.encoding
  end

  def test_upcase_matches_original_for_large_ascii_input
    input = ('abc123xyz! ' * 1_000)

    assert_equal input.original_upcase, input.upcase
  end

  def test_unicode_input_falls_back_to_ruby_behavior
    input = 'Straße Καλημέρα déjà vu'

    assert_equal input.original_upcase, input.upcase
  end

  def test_unicode_after_ascii_chunks_falls_back_to_ruby_behavior
    input = ('a' * 128) + 'é' + ('mixed123! ' * 16)

    assert_equal input.original_upcase, input.upcase
  end

  def test_instruction_set_reports_known_value
    assert_includes %w[AVX2 AVX SSE2 NEON DEFAULT], SIMDStringUpcase.instruction_set
  end

  def test_apple_silicon_uses_neon
    skip unless RUBY_PLATFORM.match?(/arm64-darwin|aarch64-darwin/)

    assert_equal 'NEON', SIMDStringUpcase.instruction_set
  end
end
