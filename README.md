# Ruby SIMD String#upcase

## Overview

`simd_string_upcase` is a Ruby gem that replaces the default `String#upcase` method a SIMD-optimized version for improved performance.

## Installation

Add this line to your application's Gemfile:

```ruby
gem 'simd_string_upcase'
```

And then execute:

```sh
bundle install
```

Or install it yourself as:

```sh
gem install simd_string_upcase
```

## Usage

After installing the gem, simply require it in your Ruby code. The `String#upcase` method will be automatically overridden to use the SIMD-optimized version.

```ruby
require 'simd_string_upcase'

str = "Hello, world!"
puts str.upcase             # This will use the SIMD-optimized version
puts str.original_upcase    # This will use the original CRuby implementation
```

## How it Works

This gem provides a drop-in replacement for the `String#upcase` method in Ruby.
It uses SIMD (Single Instruction, Multiple Data) instructions to process multiple characters in a
 string simultaneously, resulting in faster length computation compared to the default implementation.

## Performance

The SIMD-optimized methods can significantly improve performance, particularly for large strings.
By utilizing the parallel processing capabilities of modern CPUs, these methods reduce the number of 
instructions needed to compute the length of a string.

### Benchmarks

Using instruction set: AVX2

| String Size            | Implementation | Iterations per Second (i/s) | Comparison    |
|------------------------|----------------|-----------------------------|---------------|
| Small (2,000 chars)    | Ruby #upcase   | 37,196.0 i/s                |               |
|                        | SIMD #upcase   | 514,957.2 i/s               | 13.84x faster |
| Medium (200,000 chars) | Ruby #upcase   | 358.7 i/s                   |               |
|                        | SIMD #upcase   | 5,372.2 i/s                 | 14.98x faster |
| Large (2,000,000 chars)| Ruby #upcase   | 36.8 i/s                    |               |
|                        | SIMD #upcase   | 841.0 i/s                   | 22.87x faster |

**Note:** These benchmarks were run using Ruby 3.3 on macOS with a "Skylake" Intel Core i7 6820HQ CPU, which supports AVX2.

## Compatibility

**Note:** This gem currently only works on x86 CPU architectures. Support for other architectures may be 
added in the future.

## License

The gem is available as open-source under the terms of the [MIT License](https://opensource.org/licenses/MIT).
