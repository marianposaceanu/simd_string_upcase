require 'benchmark/ips'
require 'securerandom'
require 'simd_string_upcase'

# Output the used instruction set
puts "Using instruction set: #{SIMDStringUpcase.instruction_set}"


# Generate random strings for testing
def generate_random_string(length)
  chars = [('a'..'z'), ('A'..'Z'), ('0'..'9')].map(&:to_a).flatten +
    [' ', '!', '@', '#', '$', '%', '^', '&', '*', '(', ')', '-', '_', '=', '+', '[', ']', '{', '}', '|', ';', ':', '"', "'", '<', '>', ',', '.', '?', '/']
  (0...length).map { chars[SecureRandom.random_number(chars.length)] }.join
end

small_string   = generate_random_string(2_000)
medium_string  = generate_random_string(200_000)
large_string   = generate_random_string(2_000_000)
unicode_string = "α" * 1_000 + "β" * 1_000  # Example with Unicode characters


# Verify that all methods produce the same result for ASCII strings
ruby_result = small_string.original_upcase
simd_result = small_string.upcase

if ruby_result != simd_result
  puts "Error: Results don't match. Ruby: #{ruby_result}, SIMD: #{simd_result}"
  exit
end

# Benchmark with different string sizes for ASCII strings
[small_string, medium_string, large_string].each_with_index do |test_string, index|
  size = case index
         when 0 then "Small (2,000 chars)"
         when 1 then "Medium (200,000 chars)"
         when 2 then "Large (2,000,000 chars)"
         end

  puts "\nBenchmark for string size: #{size}"
  Benchmark.ips do |x|
    x.config(time: 5, warmup: 2)

    x.report("Ruby #upcase:") { test_string.original_upcase }
    x.report("SIMD #upcase:") { test_string.upcase }

    x.compare!
  end
end

# Test and compare Unicode string handling
# puts "Testing Unicode string:"
# ruby_unicode_result = unicode_string.original_upcase
# simd_unicode_result = unicode_string.upcase

# if ruby_unicode_result != simd_unicode_result
#   puts "Error: Unicode results don't match. Ruby: #{ruby_unicode_result}, SIMD: #{simd_unicode_result}"
# else
#   puts "Unicode string upcase results match."
# end
