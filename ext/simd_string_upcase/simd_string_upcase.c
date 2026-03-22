#include <ruby.h>
#include <ruby/encoding.h>
#include <stdint.h>

#if defined(__x86_64__) || defined(_M_X64)
#include <emmintrin.h>  // For SSE2 intrinsics
#include <immintrin.h>  // For AVX and AVX2 intrinsics
#endif

#if defined(__aarch64__) || defined(__arm64__)
#include <arm_neon.h>
#endif

#ifdef __APPLE__
#include <sys/types.h>
#include <sys/sysctl.h>
#elif defined(__x86_64__) || defined(_M_X64)
#include <cpuid.h>
#endif

#if defined(__x86_64__) || defined(_M_X64)
static int has_sse2 = 0, has_avx = 0, has_avx2 = 0;
#endif

#if defined(__aarch64__) || defined(__arm64__)
static int has_neon = 0;
#endif

static ID id_original_upcase;

static VALUE allocate_result_string(VALUE str) {
    VALUE result = rb_str_new(NULL, RSTRING_LEN(str));

    rb_enc_copy(result, str);

    return result;
}

#ifdef __APPLE__
// Function to detect CPU features on macOS
static void detect_cpu_features() {
#if defined(__aarch64__) || defined(__arm64__)
    size_t neon_len = sizeof(has_neon);

    if (sysctlbyname("hw.optional.neon", &has_neon, &neon_len, NULL, 0) != 0) {
        has_neon = 1;
    }
#else
    size_t sse2_len = sizeof(has_sse2);
    size_t avx_len = sizeof(has_avx);
    size_t avx2_len = sizeof(has_avx2);

    sysctlbyname("hw.optional.sse2", &has_sse2, &sse2_len, NULL, 0);
    sysctlbyname("hw.optional.avx1_0", &has_avx, &avx_len, NULL, 0);
    sysctlbyname("hw.optional.avx2_0", &has_avx2, &avx2_len, NULL, 0);
#endif
}
#elif defined(__x86_64__) || defined(_M_X64)
// Function to detect CPU features on Linux
static void detect_cpu_features() {
    unsigned int eax, ebx, ecx, edx;

    if (__get_cpuid(1, &eax, &ebx, &ecx, &edx)) {
        has_sse2 = edx & (1 << 26);
        has_avx = ecx & (1 << 28);
    }

    if (has_avx && __get_cpuid_count(7, 0, &eax, &ebx, &ecx, &edx)) {
        has_avx2 = ebx & (1 << 5);
    }
}
#else
static void detect_cpu_features() {
#if defined(__aarch64__) || defined(__arm64__)
    has_neon = 1;
#endif
}
#endif

#if defined(__x86_64__) || defined(_M_X64)
static VALUE upcase_sse2(VALUE self, VALUE str) {
    char *text = StringValuePtr(str);
    long len = RSTRING_LEN(str);
    VALUE result = allocate_result_string(str);
    char *res_text = StringValuePtr(result);

    __m128i lower_a = _mm_set1_epi8('a');
    __m128i lower_z = _mm_set1_epi8('z');
    __m128i diff = _mm_set1_epi8('a' - 'A');

    long i;
    for (i = 0; i <= len - 16; i += 16) {
        __m128i chunk = _mm_loadu_si128((__m128i*)(text + i));
        __m128i mask_a = _mm_cmpgt_epi8(chunk, _mm_sub_epi8(lower_a, _mm_set1_epi8(1)));
        __m128i mask_z = _mm_cmplt_epi8(chunk, _mm_add_epi8(lower_z, _mm_set1_epi8(1)));
        __m128i mask = _mm_and_si128(mask_a, mask_z);
        chunk = _mm_sub_epi8(chunk, _mm_and_si128(mask, diff));
        _mm_storeu_si128((__m128i*)(res_text + i), chunk);
    }

    for (; i < len; i++) {
        if (text[i] >= 'a' && text[i] <= 'z') {
            res_text[i] = text[i] - ('a' - 'A');
        } else {
            res_text[i] = text[i];
        }
    }

    return result;
}

static VALUE upcase_avx1(VALUE self, VALUE str) {
    char *text = StringValuePtr(str);
    long len = RSTRING_LEN(str);
    VALUE result = allocate_result_string(str);
    char *res_text = StringValuePtr(result);

    __m256i lower_a = _mm256_set1_epi8('a');
    __m256i lower_z = _mm256_set1_epi8('z');
    __m256i diff = _mm256_set1_epi8('a' - 'A');

    long i;
    for (i = 0; i <= len - 32; i += 32) {
        __m256i chunk = _mm256_loadu_si256((__m256i*)(text + i));
        __m256i mask_a = _mm256_cmpgt_epi8(chunk, _mm256_sub_epi8(lower_a, _mm256_set1_epi8(1)));
        __m256i mask_z = _mm256_cmpgt_epi8(_mm256_set1_epi8('z' + 1), chunk);
        __m256i mask = _mm256_and_si256(mask_a, mask_z);
        chunk = _mm256_sub_epi8(chunk, _mm256_and_si256(mask, diff));
        _mm256_storeu_si256((__m256i*)(res_text + i), chunk);
    }

    for (; i < len; i++) {
        if (text[i] >= 'a' && text[i] <= 'z') {
            res_text[i] = text[i] - 32;
        } else {
            res_text[i] = text[i];
        }
    }

    return result;
}

static VALUE upcase_avx2(VALUE self, VALUE str) {
    char *text = StringValuePtr(str);
    long len = RSTRING_LEN(str);
    VALUE result = allocate_result_string(str);
    char *res_text = StringValuePtr(result);

    __m256i lower_a = _mm256_set1_epi8('a');
    __m256i lower_z = _mm256_set1_epi8('z');
    __m256i diff = _mm256_set1_epi8('a' - 'A');

    long i;
    for (i = 0; i <= len - 64; i += 64) {
        __m256i chunk1 = _mm256_loadu_si256((__m256i*)(text + i));
        __m256i chunk2 = _mm256_loadu_si256((__m256i*)(text + i + 32));

        __m256i mask1_a = _mm256_cmpgt_epi8(chunk1, _mm256_sub_epi8(lower_a, _mm256_set1_epi8(1)));
        __m256i mask1_z = _mm256_cmpgt_epi8(_mm256_set1_epi8('z' + 1), chunk1);
        __m256i mask1 = _mm256_and_si256(mask1_a, mask1_z);

        __m256i mask2_a = _mm256_cmpgt_epi8(chunk2, _mm256_sub_epi8(lower_a, _mm256_set1_epi8(1)));
        __m256i mask2_z = _mm256_cmpgt_epi8(_mm256_set1_epi8('z' + 1), chunk2);
        __m256i mask2 = _mm256_and_si256(mask2_a, mask2_z);

        chunk1 = _mm256_sub_epi8(chunk1, _mm256_and_si256(mask1, diff));
        chunk2 = _mm256_sub_epi8(chunk2, _mm256_and_si256(mask2, diff));

        _mm256_storeu_si256((__m256i*)(res_text + i), chunk1);
        _mm256_storeu_si256((__m256i*)(res_text + i + 32), chunk2);
    }

    for (; i < len; i++) {
        if (text[i] >= 'a' && text[i] <= 'z') {
            res_text[i] = text[i] - 32;
        } else {
            res_text[i] = text[i];
        }
    }

    return result;
}
#endif

#if defined(__aarch64__) || defined(__arm64__)
static VALUE upcase_neon(VALUE self, VALUE str) {
    const uint8_t *text = (const uint8_t *)StringValuePtr(str);
    long len = RSTRING_LEN(str);
    VALUE result = allocate_result_string(str);
    uint8_t *res_text = (uint8_t *)StringValuePtr(result);

    uint8x16_t lower_a = vdupq_n_u8('a');
    uint8x16_t lower_z = vdupq_n_u8('z');
    uint8x16_t diff = vdupq_n_u8('a' - 'A');

    long i;
    for (i = 0; i <= len - 16; i += 16) {
        uint8x16_t chunk = vld1q_u8(text + i);
        uint8x16_t mask_a = vcgeq_u8(chunk, lower_a);
        uint8x16_t mask_z = vcleq_u8(chunk, lower_z);
        uint8x16_t mask = vandq_u8(mask_a, mask_z);

        chunk = vsubq_u8(chunk, vandq_u8(mask, diff));
        vst1q_u8(res_text + i, chunk);
    }

    for (; i < len; i++) {
        if (text[i] >= 'a' && text[i] <= 'z') {
            res_text[i] = text[i] - ('a' - 'A');
        } else {
            res_text[i] = text[i];
        }
    }

    return result;
}
#endif

static VALUE simd_upcase(VALUE self, VALUE str) {
    // Check if the string is ASCII only
    long len = RSTRING_LEN(str);
    const unsigned char *text = (const unsigned char *)StringValuePtr(str);

    for (long i = 0; i < len; i++) {
        if (text[i] > 127) {
            // Fallback to default Ruby implementation for non-ASCII characters
            return rb_funcall(str, id_original_upcase, 0);
        }
    }

    // Use SIMD optimized version for ASCII-only strings
#if defined(__aarch64__) || defined(__arm64__)
    if (has_neon) {
        return upcase_neon(self, str);
    }
#endif

#if defined(__x86_64__) || defined(_M_X64)
    if (has_avx2) {
        return upcase_avx2(self, str);
    } else if (has_avx) {
        return upcase_avx1(self, str);
    } else if (has_sse2) {
        return upcase_sse2(self, str);
    }
#endif

    // Fallback to default Ruby implementation
    return rb_funcall(str, id_original_upcase, 0);
}

// Function to return the used instruction set
static VALUE get_instruction_set(VALUE self) {
#if defined(__aarch64__) || defined(__arm64__)
    if (has_neon) {
        return rb_str_new_cstr("NEON");
    }
#endif

#if defined(__x86_64__) || defined(_M_X64)
    if (has_avx2) {
        return rb_str_new_cstr("AVX2");
    } else if (has_avx) {
        return rb_str_new_cstr("AVX");
    } else if (has_sse2) {
        return rb_str_new_cstr("SSE2");
    } else {
        return rb_str_new_cstr("DEFAULT");
    }
#else
    return rb_str_new_cstr("DEFAULT");
#endif
}

void Init_simd_string_upcase(void) {
    detect_cpu_features();
    id_original_upcase = rb_intern("original_upcase");
    VALUE module = rb_define_module("SIMDStringUpcase");
    rb_define_singleton_method(module, "upcase", simd_upcase, 1);
    rb_define_singleton_method(module, "instruction_set", get_instruction_set, 0);
}
