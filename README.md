# Ruby SIMD String#upcase

## Overview

`simd_string_upcase` is a native-extension gem that replaces the default `String#upcase` method with a SIMD-optimized version for improved ASCII performance.

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

> [!WARNING]
> Requiring this gem globally monkey-patches `String#upcase` for the entire process and adds
> `String#original_upcase`. The replacement supports the no-argument form shown above. Review
> compatibility with other gems and code that prepends, aliases, or calls `String#upcase` with
> options before using it in an application.

## How it Works

This gem provides a drop-in replacement for the `String#upcase` method in Ruby.
It uses SIMD (Single Instruction, Multiple Data) instructions to process multiple characters in a
string simultaneously. Strings shorter than 64 bytes, non-ASCII strings, and machines without a
supported SIMD instruction set fall back to CRuby's original implementation.

## Performance

The SIMD-optimized methods can significantly improve performance, particularly for large strings.
By utilizing the parallel processing capabilities of modern CPUs, these methods reduce the number of 
instructions needed to uppercase an ASCII string.

### Benchmarks

#### Intel macOS (AVX2)

Using instruction set: AVX2

| String Size             | Implementation | Iterations per Second (i/s) | Comparison    |
|-------------------------|----------------|-----------------------------|---------------|
| Small (2,000 chars)     | Ruby #upcase   | 37,196.0 i/s                |               |
|                         | SIMD #upcase   | 514,957.2 i/s               | 13.84x faster |
| Medium (200,000 chars)  | Ruby #upcase   | 358.7 i/s                   |               |
|                         | SIMD #upcase   | 5,372.2 i/s                 | 14.98x faster |
| Large (2,000,000 chars) | Ruby #upcase   | 36.8 i/s                    |               |
|                         | SIMD #upcase   | 841.0 i/s                   | 22.87x faster |

**Note:** These benchmarks were run using Ruby 3.3 on macOS with a "Skylake" Intel Core i7
6820HQ CPU, which supports AVX2.

#### Apple Silicon macOS (NEON)

Using instruction set: NEON

| String Size             | Implementation | Iterations per Second (i/s) | Comparison   |
|-------------------------|----------------|-----------------------------|--------------|
| Small (2,000 chars)     | Ruby #upcase   | 1,645,781.1 i/s             |              |
|                         | SIMD #upcase   | 5,325,565.5 i/s             | 3.24x faster |
| Medium (200,000 chars)  | Ruby #upcase   | 7,434.5 i/s                 |              |
|                         | SIMD #upcase   | 177,237.2 i/s               | 23.84x faster |
| Large (2,000,000 chars) | Ruby #upcase   | 272.6 i/s                   |              |
|                         | SIMD #upcase   | 19,756.9 i/s                | 72.47x faster |

**Note:** These benchmarks were run using Ruby 3.4.2 on macOS with an Apple M4 CPU.

Benchmark results are hardware-, compiler-, Ruby-version-, string-content-, and workload-specific.
They are historical measurements, not performance guarantees. Run `bundle exec ruby
tests/benchmarks.rb` on the target system before making deployment decisions.

## Compatibility

The supported and tested combinations are:

| Ruby | Architecture | SIMD path | Status |
|------|--------------|-----------|--------|
| 3.3 | x86_64 | SSE2, AVX, or AVX2 | CI matrix and Intel benchmark |
| 3.4 | x86_64 | SSE2, AVX, or AVX2 | CI matrix |
| 4.0 | x86_64 | SSE2, AVX, or AVX2 | CI matrix |
| 4.0 | arm64 macOS | NEON | CI matrix and locally tested on 4.0.6 |
| 3.4.2 | arm64 macOS (Apple M4) | NEON | Benchmark evidence |

Ruby 3.3 or newer is required. The native implementation targets x86_64 and AArch64/arm64;
other architectures are not supported. x86_64 execution requires at least SSE2 for acceleration,
while AArch64 requires NEON. Runtime feature detection selects the available implementation and
otherwise falls back to CRuby.

Installation builds C code locally and therefore requires a C compiler, `make`, Ruby development
headers, and the platform SIMD intrinsic headers. On macOS, install the Xcode Command Line Tools.
On Linux, install the distribution's compiler toolchain and Ruby development package. This gem does
not currently publish precompiled native gems.

The test matrix covers current CRuby releases on GitHub-hosted Linux x86_64 runners and Ruby 4.0 on
an Apple Silicon macOS runner. Other operating systems, compilers, Ruby implementations, and CPUs
remain unverified.

## License

The gem is available as open-source under the terms of the [MIT License](https://opensource.org/licenses/MIT).
