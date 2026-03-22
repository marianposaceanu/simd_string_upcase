require 'mkmf'

ARM64_PLATFORM = /arm64|aarch64/
DARWIN_PLATFORM = /darwin/

def arm64_platform?
  RUBY_PLATFORM.match?(ARM64_PLATFORM)
end

def darwin_platform?
  RUBY_PLATFORM.match?(DARWIN_PLATFORM)
end

def configure_arm64(platform_name)
  have_header('arm_neon.h')
  puts "#{platform_name} detected. Enabling NEON support."
end

def configure_intel_macos
  have_header('emmintrin.h')
  have_header('immintrin.h')
  $CFLAGS << ' -mavx -mavx2'
  puts 'Intel macOS detected. Forcing AVX and AVX2 support.'
end

def configure_x86
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

if darwin_platform? && arm64_platform?
  configure_arm64('Apple Silicon')
elsif darwin_platform?
  configure_intel_macos
elsif arm64_platform?
  configure_arm64('ARM64')
else
  configure_x86
end

create_makefile('simd_string_upcase/simd_string_upcase')
