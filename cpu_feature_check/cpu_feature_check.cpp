#include <iostream>

#include <cstdint>

#define CPUID_FEATURES_SSE2 (1 << 26)

#ifdef _MSC_VER
#define COMPILER_MSVC 1
#elif defined(__clang__)
#define COMPILER_CLANG 1
#elif defined(__GNUC__)
#define COMPILER_GCC 1
#endif

#ifdef __EMSCRIPTEN__

void __cpuid(int data[4], int function_id)
{
    data[0] = data[1] = data[2] = data[3] = 0;
    if (function_id == 0)
    {
        data[0] = 1; // Basic info supported
        return;
    }
    else if (function_id == 1)
    {
        // Mimic fields reported by x86 cpuid instruction:
        // https://learn.microsoft.com/en-us/cpp/intrinsics/cpuid-cpuidex?view=msvc-170
#ifdef __SSE__
        data[3] |= 1 << 25;
#endif
#ifdef __SSE2__
        data[3] |= 1 << 26;
#endif
#ifdef __SSE3__
        data[2] |= 1;
#endif
#ifdef __SSSE3__
        data[2] |= 1 << 9;
#endif
#ifdef __SSE_4_1__
        data[2] |= 1 << 19;
#endif
#ifdef __SSE_4_2__
        data[2] |= 1 << 20;
#endif
    }
}

#elif COMPILER_MSVC

// define __cpuid intrinsic
#include <intrin.h>

#elif COMPILER_CLANG || COMPILER_GCC

#if defined(__x86_64__)
#   define __cpuid(array, func) \
    { \
        __asm__ __volatile__("pushq %%rbx     \n\t" /* save %rbx */ \
                             "cpuid            \n\t" \
                             "popq %%rbx \n\t" /* restore the old %rbx */ \
                             : "=a"(array[0]), "=D"(array[1]), "=c"(array[2]), "=d"(array[3]) \
                             : "a"(func) \
                             : "cc");\
    }
#else
#   define __cpuid(array, func) \
    { \
        __asm__ __volatile__("xchg %%ebx, %%edi      \n\t" /* save %ebx */ \
                            "cpuid            \n\t" \
                            "xchg %%ebx, %%edi   \n\t" /* restore the old %ebx */ \
                            : "=a"(array[0]), "=D"(array[1]), "=c"(array[2]), "=d"(array[3]) \
                            : "a"(func) \
                            : "cc");\
    }
#endif //defined(__x86_64__)

#else
#define __cpuid(a, b)
#endif

static inline uint64_t xgetbv_impl()
{
#   ifdef __EMSCRIPTEN__
    return 0;
#   elif COMPILER_CLANG || COMPILER_GCC
    uint32_t eax, edx;

    __asm __volatile(
    ".byte 0x0f, 0x01, 0xd0" // xgetbv instruction isn't supported by some older assemblers, so just emit it raw
        : "=a" (eax), "=d" (edx) : "c" (0)
        );

    return ((UInt64)edx << 32) | eax;
#   elif COMPILER_MSVC
    return _xgetbv(_XCR_XFEATURE_ENABLED_MASK);
#   else
    return 0;
#   endif
}

static void cpuid_ex_impl(uint32_t eax, uint32_t ecx, uint32_t* abcd)
{
#ifdef __EMSCRIPTEN__
    abcd[0] = 0; abcd[1] = 0; abcd[2] = 0; abcd[3] = 0;
#elif COMPILER_MSVC

    __cpuidex((int*)abcd, eax, ecx);

#else
    uint32_t ebx = 0, edx = 0;
#if defined(__i386__) && defined(__PIC__)
    // for PIC under 32 bit: EBX can't be modified
    __asm__("movl %%ebx, %%edi \n\t cpuid \n\t xchgl %%ebx, %%edi" : "=D" (ebx), "+a" (eax), "+c" (ecx), "=d" (edx));
#else
    __asm__("cpuid" : "+b" (ebx), "+a" (eax), "+c" (ecx), "=d" (edx));
#endif
    abcd[0] = eax; abcd[1] = ebx; abcd[2] = ecx; abcd[3] = edx;
#endif
}

void cpu_info_check()
{
    int data[4] = { 0 };

    // Add more code here to extract vendor string or what ever is needed
    __cpuid(data, 0);
    unsigned int cpuData0 = data[0];
    unsigned int cpuInfo2 = 0;
    unsigned int cpuIDFeatures = 0;
    if (cpuData0 >= 1)
    {
        __cpuid(data, 1);
        cpuInfo2 = data[2];
        cpuIDFeatures = data[3];
    }

    // SSE2 support
    bool m_IsSSE2Supported = (cpuIDFeatures & CPUID_FEATURES_SSE2) != 0;

    // SSE 3.x
    bool m_IsSSE3Supported = ((cpuInfo2 & (1 << 0)) != 0);
    bool m_IsSupplementalSSE3Supported = ((cpuInfo2 & (1 << 9)) != 0);

    // SSE 4.x support
    bool m_IsSSE41Supported = ((cpuInfo2 & (1 << 19)) != 0);
    bool m_IsSSE42Supported = ((cpuInfo2 & (1 << 20)) != 0);

    // AVX support
    bool m_IsAVXSupported =
        ((cpuInfo2 & (1 << 28)) != 0) && // AVX support in CPU
        ((cpuInfo2 & (1 << 27)) != 0) && // OS support for AVX (XSAVE/XRESTORE on context switches)
        ((xgetbv_impl() & 6) == 6); // XMM & YMM registers will be preserved on context switches

    if (m_IsAVXSupported)
    {
        if (cpuData0 >= 7)
        {
            uint32_t regs7[4] = { 0 };
            cpuid_ex_impl(0x7, 0, regs7);
            bool m_IsAVX2Supported = ((regs7[1] & (1 << 5)) != 0);
            bool m_IsAVX512Supported = ((regs7[1] & (1 << 16)) != 0);
        }
    }

    bool m_IsFP16CSupported = ((cpuInfo2 & (1 << 29)) != 0);
    bool m_IsFMASupported = ((cpuInfo2 & (1 << 12)) != 0);

    bool m_IsAdvancedBitManipulationSupported = ((cpuInfo2 & (1 << 23)) != 0);
}

int main()
{
    cpu_info_check();
}
