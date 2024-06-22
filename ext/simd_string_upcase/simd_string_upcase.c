#include <ruby.h>
#include <emmintrin.h>  // For SSE2 intrinsics
#include <immintrin.h>  // For AVX and AVX2 intrinsics

#ifdef __APPLE__
#include <sys/types.h>
#include <sys/sysctl.h>
#else
#include <cpuid.h>
#endif

static int has_sse2 = 0, has_avx = 0, has_avx2 = 0;

#ifdef __APPLE__
// Function to detect CPU features on macOS
static void detect_cpu_features() {
    size_t sse2_len = sizeof(has_sse2);
    size_t avx_len = sizeof(has_avx);
    size_t avx2_len = sizeof(has_avx2);

    sysctlbyname("hw.optional.sse2", &has_sse2, &sse2_len, NULL, 0);
    sysctlbyname("hw.optional.avx1_0", &has_avx, &avx_len, NULL, 0);
    sysctlbyname("hw.optional.avx2_0", &has_avx2, &avx2_len, NULL, 0);
}
#else
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
#endif

static VALUE upcase_sse2(VALUE self, VALUE str) {
    char *text = StringValuePtr(str);
    long len = RSTRING_LEN(str);
    VALUE result = rb_str_new(NULL, len);
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
    VALUE result = rb_str_new(NULL, len);
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
    VALUE result = rb_str_new(NULL, len);
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

static VALUE simd_upcase(VALUE self, VALUE str) {
    // Check if the string is ASCII only
    long len = RSTRING_LEN(str);
    char *text = StringValuePtr(str);
    for (long i = 0; i < len; i++) {
        if ((unsigned char)text[i] > 127) {
            // Fallback to default Ruby implementation for non-ASCII characters
            return rb_funcall(str, rb_intern("upcase"), 0);
        }
    }

    // Use SIMD optimized version for ASCII-only strings
    if (has_avx2) {
        return upcase_avx2(self, str);
    } else if (has_avx) {
        return upcase_avx1(self, str);
    } else if (has_sse2) {
        return upcase_sse2(self, str);
    } else {
        // Fallback to default Ruby implementation
        return rb_funcall(str, rb_intern("upcase"), 0);
    }
}

// Function to return the used instruction set
static VALUE get_instruction_set(VALUE self) {
    if (has_avx2) {
        return rb_str_new_cstr("AVX2");
    } else if (has_avx) {
        return rb_str_new_cstr("AVX");
    } else if (has_sse2) {
        return rb_str_new_cstr("SSE2");
    } else {
        return rb_str_new_cstr("DEFAULT");
    }
}

void Init_simd_string_upcase(void) {
    detect_cpu_features();
    VALUE module = rb_define_module("SIMDStringUpcase");
    rb_define_singleton_method(module, "upcase", simd_upcase, 1);
    rb_define_singleton_method(module, "instruction_set", get_instruction_set, 0);
}
