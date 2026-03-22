require 'mkmf'

if RUBY_PLATFORM =~ /darwin/ && RUBY_PLATFORM =~ /arm64|aarch64/
  abort 'ARM NEON headers are required to build on Apple Silicon.' unless have_header('arm_neon.h')

  puts 'Apple Silicon detected. Enabling NEON support.'
elsif RUBY_PLATFORM =~ /darwin/
  have_header('emmintrin.h')
  have_header('immintrin.h')
  $CFLAGS << ' -mavx -mavx2'
  puts 'Intel macOS detected. Forcing AVX and AVX2 support.'
else
  if RUBY_PLATFORM =~ /arm64|aarch64/
    abort 'ARM NEON headers are required to build on ARM64.' unless have_header('arm_neon.h')

    puts 'ARM64 detected. Enabling NEON support.'
  else
    have_header('emmintrin.h')
    have_header('immintrin.h')

    have_sse2 = have_macro('__SSE2__', 'emmintrin.h')
    have_avx = have_macro('__AVX__', 'immintrin.h')
    have_avx2 = have_macro('__AVX2__', 'immintrin.h')

    if have_sse2
      $CFLAGS << ' -msse2'
      puts 'SSE2 support detected and enabled.'
    else
      puts 'Warning: SSE2 not supported. The SSE2 version may not work correctly.'
    end

    if have_avx
      $CFLAGS << ' -mavx'
      puts 'AVX support detected and enabled.'
    else
      puts 'Warning: AVX not supported. The AVX1 version may not work correctly.'
    end

    if have_avx2
      $CFLAGS << ' -mavx2'
      puts 'AVX2 support detected and enabled.'
    else
      puts 'Warning: AVX2 not supported. The AVX2 version may not work correctly.'
    end
  end
end

create_makefile('simd_string_upcase/simd_string_upcase')
