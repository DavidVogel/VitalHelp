#pragma once

#include <cstdint>
#include <climits>
#include <cstdlib>

/**
 * @file poly_values.h
 * @brief Defines SIMD-based vectorized integer and floating-point types (poly_int and poly_float)
 *        along with associated operations for use in polyphonic synthesis.
 *
 * This header provides structures and functions for performing vectorized arithmetic,
 * logical, and comparison operations on integers and floating-point numbers. It uses
 * SIMD intrinsics (SSE2, AVX2, or NEON) depending on the platform. These types and
 * functions are primarily used to accelerate computations across multiple "voices"
 * or polyphonic units simultaneously in the Vital synthesizer.
 */

#if VITAL_AVX2
  #define VITAL_AVX2 1
  static_assert(false, "AVX2 is not supported yet.");
#elif __SSE2__
  #define VITAL_SSE2 1
#elif defined(__ARM_NEON__) || defined(__ARM_NEON)
  #define VITAL_NEON 1
#else
  static_assert(false, "No SIMD Intrinsics found which are necessary for compilation");
#endif

#if VITAL_SSE2
  #include <immintrin.h>
#elif VITAL_NEON
  #include <arm_neon.h>
#endif

#if !defined(force_inline)
#if defined (_MSC_VER)
#define force_inline __forceinline
  #define vector_call __vectorcall
#else
  #define force_inline inline __attribute__((always_inline))
  #define vector_call
#endif
#endif

namespace vital {

  /**
   * @struct poly_int
   * @brief Represents a vector of integer values using SIMD instructions.
   *
   * Depending on the available instruction set (AVX2, SSE2, NEON), the vector
   * width differs. This struct provides basic arithmetic, bitwise, and comparison
   * operations on these SIMD vectors.
   */
  struct poly_int {
#if VITAL_AVX2
    static constexpr size_t kSize = 8; ///< Number of elements in the SIMD register
    typedef __m256i simd_type;        ///< Underlying SIMD type
#elif VITAL_SSE2
    static constexpr size_t kSize = 4; ///< Number of elements in the SIMD register
    typedef __m128i simd_type;        ///< Underlying SIMD type
#elif VITAL_NEON
    static constexpr size_t kSize = 4; ///< Number of elements in the SIMD register
    typedef uint32x4_t simd_type;     ///< Underlying SIMD type
#endif

    /**
     * @union scalar_simd_union
     * @brief Helper union for copying between a scalar array and a SIMD type.
     */
    union scalar_simd_union {
      int32_t scalar[kSize]; ///< Access data in scalar form
      simd_type simd;        ///< Access data in SIMD form
    };

    /**
     * @union simd_scalar_union
     * @brief Helper union for copying between a SIMD type and a scalar array.
     */
    union simd_scalar_union {
      simd_type simd;        ///< Access data in SIMD form
      int32_t scalar[kSize]; ///< Access data in scalar form
    };

    static constexpr uint32_t kFullMask      = (unsigned int)-1;   ///< All bits set
    static constexpr uint32_t kSignMask      = 0x80000000;         ///< Sign bit mask
    static constexpr uint32_t kNotSignMask   = kFullMask ^ kSignMask; ///< Inverted sign bit mask

    /**
     * @brief Initializes a SIMD register with the same integer repeated.
     * @param scalar Integer value to broadcast across the SIMD register.
     * @return Initialized SIMD register.
     */
    static force_inline simd_type vector_call init(uint32_t scalar) {
#if VITAL_AVX2
      return _mm256_set1_epi32((int32_t)scalar);
#elif VITAL_SSE2
      return _mm_set1_epi32((int32_t)scalar);
#elif VITAL_NEON
      return vdupq_n_u32(scalar);
#endif
    }

    /**
     * @brief Loads integer values from memory into a SIMD register.
     * @param memory Pointer to memory location where integer data is stored.
     * @return Loaded SIMD register.
     */
    static force_inline simd_type vector_call load(const uint32_t* memory) {
#if VITAL_AVX2
      return _mm256_loadu_si256((const __m256i*)scalar);
#elif VITAL_SSE2
      return _mm_loadu_si128((const __m128i*)memory);
#elif VITAL_NEON
      return vld1q_u32(memory);
#endif
    }

    /**
     * @brief Adds two SIMD integer registers.
     * @param one First operand.
     * @param two Second operand.
     * @return Sum of the two operands.
     */
    static force_inline simd_type vector_call add(simd_type one, simd_type two) {
#if VITAL_AVX2
      return _mm256_add_epi32(one, two);
#elif VITAL_SSE2
      return _mm_add_epi32(one, two);
#elif VITAL_NEON
      return vaddq_u32(one, two);
#endif
    }

    /**
     * @brief Subtracts one SIMD integer register from another.
     * @param one First operand.
     * @param two Second operand.
     * @return Result of `one - two`.
     */
    static force_inline simd_type vector_call sub(simd_type one, simd_type two) {
#if VITAL_AVX2
      return _mm256_sub_epi32(one, two);
#elif VITAL_SSE2
      return _mm_sub_epi32(one, two);
#elif VITAL_NEON
      return vsubq_u32(one, two);
#endif
    }

    /**
     * @brief Negates a SIMD integer register.
     * @param value SIMD register to negate.
     * @return Negation of the input register.
     */
    static force_inline simd_type vector_call neg(simd_type value) {
#if VITAL_AVX2
      return _mm256_sub_epi32(_mm256_set1_epi32(0), value);
#elif VITAL_SSE2
      return _mm_sub_epi32(_mm_set1_epi32(0), value);
#elif VITAL_NEON
      return vmulq_n_u32(value, -1);
#endif
    }

    /**
     * @brief Multiplies two SIMD integer registers element-wise.
     * @param one First operand.
     * @param two Second operand.
     * @return Product of the two operands.
     */
    static force_inline simd_type vector_call mul(simd_type one, simd_type two) {
#if VITAL_AVX2
      return _mm256_mul_epi32(one, two);
#elif VITAL_SSE2
      // SSE2 does not have a direct epi32 multiply, so we emulate it:
      simd_type mul0_2 = _mm_mul_epu32(one, two);
      simd_type mul1_3 = _mm_mul_epu32(
        _mm_shuffle_epi32(one, _MM_SHUFFLE(2, 3, 0, 1)),
        _mm_shuffle_epi32(two, _MM_SHUFFLE(2, 3, 0, 1)));
      return _mm_unpacklo_epi32(
        _mm_shuffle_epi32(mul0_2, _MM_SHUFFLE (0, 0, 2, 0)),
        _mm_shuffle_epi32(mul1_3, _MM_SHUFFLE (0, 0, 2, 0)));
#elif VITAL_NEON
      return vmulq_u32(one, two);
#endif
    }

    /**
     * @brief Bitwise AND of a SIMD integer register with another.
     * @param value The operand to be masked.
     * @param mask The mask to apply.
     * @return Bitwise AND of value and mask.
     */
    static force_inline simd_type vector_call bitAnd(simd_type value, simd_type mask) {
#if VITAL_AVX2
      return _mm256_and_si256(value, mask);
#elif VITAL_SSE2
      return _mm_and_si128(value, mask);
#elif VITAL_NEON
      return vandq_u32(value, mask);
#endif
    }

    /**
     * @brief Bitwise OR of a SIMD integer register with another.
     * @param value The operand to be combined.
     * @param mask The mask to apply.
     * @return Bitwise OR of value and mask.
     */
    static force_inline simd_type vector_call bitOr(simd_type value, simd_type mask) {
#if VITAL_AVX2
      return _mm256_or_si256(value, mask);
#elif VITAL_SSE2
      return _mm_or_si128(value, mask);
#elif VITAL_NEON
      return vorrq_u32(value, mask);
#endif
    }

    /**
     * @brief Bitwise XOR of a SIMD integer register with another.
     * @param value The operand to be combined.
     * @param mask The mask to apply.
     * @return Bitwise XOR of value and mask.
     */
    static force_inline simd_type vector_call bitXor(simd_type value, simd_type mask) {
#if VITAL_AVX2
      return _mm256_xor_si256(value, mask);
#elif VITAL_SSE2
      return _mm_xor_si128(value, mask);
#elif VITAL_NEON
      return veorq_u32(value, mask);
#endif
    }

    /**
     * @brief Bitwise NOT of a SIMD integer register.
     * @param value The operand to be inverted.
     * @return Bitwise NOT of the value.
     */
    static force_inline simd_type vector_call bitNot(simd_type value) {
      return bitXor(value, init(-1));
    }

    /**
     * @brief Returns the element-wise maximum of two SIMD integer registers.
     * @param one First operand.
     * @param two Second operand.
     * @return Element-wise maximum.
     */
    static force_inline simd_type vector_call max(simd_type one, simd_type two) {
#if VITAL_AVX2
      return _mm256_max_epi32(one, two);
#elif VITAL_SSE2
      simd_type greater_than_mask = greaterThan(one, two);
      // Choose 'one' where mask is set, else 'two'
      return _mm_or_si128(_mm_and_si128(greater_than_mask, one),
                          _mm_andnot_si128(greater_than_mask, two));
#elif VITAL_NEON
      return vmaxq_u32(one, two);
#endif
    }

    /**
     * @brief Returns the element-wise minimum of two SIMD integer registers.
     * @param one First operand.
     * @param two Second operand.
     * @return Element-wise minimum.
     */
    static force_inline simd_type vector_call min(simd_type one, simd_type two) {
#if VITAL_AVX2
      return _mm256_min_epi32(one, two);
#elif VITAL_SSE2
      simd_type less_than_mask = _mm_cmpgt_epi32(two, one);
      // Choose 'one' where mask is set, else 'two'
      return _mm_or_si128(_mm_and_si128(less_than_mask, one),
                          _mm_andnot_si128(less_than_mask, two));
#elif VITAL_NEON
      return vminq_u32(one, two);
#endif
    }

    /**
     * @brief Compares two SIMD integer registers for equality, element-wise.
     * @param one First operand.
     * @param two Second operand.
     * @return Register of all bits set if equal, zero if not.
     */
    static force_inline simd_type vector_call equal(simd_type one, simd_type two) {
#if VITAL_AVX2
      return _mm256_cmpeq_epi32(one, two);
#elif VITAL_SSE2
      return _mm_cmpeq_epi32(one, two);
#elif VITAL_NEON
      return vceqq_u32(one, two);
#endif
    }

    /**
     * @brief Compares two SIMD integer registers, element-wise, for greater than.
     * @param one First operand.
     * @param two Second operand.
     * @return Register of all bits set if one[i] > two[i], zero otherwise.
     *
     * Note: For SSE2/AVX2, we handle sign by flipping the sign bit.
     */
    static force_inline simd_type vector_call greaterThan(simd_type one, simd_type two) {
#if VITAL_AVX2
      return _mm256_cmpgt_epi32(_mm256_xor_si256(one, init(kSignMask)),
                                _mm256_xor_si256(two, init(kSignMask)));
#elif VITAL_SSE2
      return _mm_cmpgt_epi32(_mm_xor_si128(one, init(kSignMask)),
                             _mm_xor_si128(two, init(kSignMask)));
#elif VITAL_NEON
      return vcgtq_u32(one, two);
#endif
    }

    /**
     * @brief Computes the sum of all elements in a SIMD integer register.
     * @param value The SIMD register.
     * @return The sum of all elements.
     */
    static force_inline uint32_t vector_call sum(simd_type value) {
#if VITAL_AVX2
      // Example logic (not fully implemented):
      // simd_type flip = _mm256_permute4x64_epi64(value, _MM_SHUFFLE(1, 0, 3, 2));
      // ...
      // return ...
      // Implementation incomplete in this code snippet.
      #error "AVX2 version not fully implemented in code snippet"
#elif VITAL_SSE2
      simd_scalar_union union_value { value };
      uint32_t total = 0;
      for (int i = 0; i < kSize; ++i)
        total += union_value.scalar[i];
      return total;
#elif VITAL_NEON
      uint32x2_t partial_sum = vpadd_u32(vget_low_u32(value), vget_high_u32(value));
      partial_sum = vpadd_u32(partial_sum, partial_sum);
      return vget_lane_u32(partial_sum, 0);
#endif
    }

    /**
     * @brief Returns a bitmask that indicates which bytes/elements in the register are non-zero.
     * @param value The SIMD register to test.
     * @return Platform-specific bitmask of set bits.
     */
    static force_inline uint32_t vector_call anyMask(simd_type value) {
#if VITAL_AVX2
      return _mm256_movemask_epi8(value);
#elif VITAL_SSE2
      return _mm_movemask_epi8(value);
#elif VITAL_NEON
      // For NEON, we typically reduce it down:
      uint32x2_t max_vals = vpmax_u32(vget_low_u32(value), vget_high_u32(value));
      max_vals = vpmax_u32(max_vals, max_vals);
      return vget_lane_u32(max_vals, 0);
#endif
    }

    /// Convenience overloads returning poly_int instead of simd_type:
    static force_inline poly_int vector_call max(poly_int one, poly_int two) {
      return max(one.value, two.value);
    }
    static force_inline poly_int vector_call min(poly_int one, poly_int two) {
      return min(one.value, two.value);
    }
    static force_inline poly_int vector_call equal(poly_int one, poly_int two) {
      return equal(one.value, two.value);
    }
    static force_inline poly_int vector_call greaterThan(poly_int one, poly_int two) {
      return greaterThan(one.value, two.value);
    }
    static force_inline poly_int vector_call lessThan(poly_int one, poly_int two) {
      return greaterThan(two.value, one.value);
    }
    static force_inline uint32_t vector_call sum(poly_int value) {
      return sum(value.value);
    }

    simd_type value; ///< The underlying SIMD register.

    /**
     * @brief Default constructor. Initializes to zero.
     */
    force_inline poly_int() noexcept { value = init(0); }

    /**
     * @brief Constructs from a raw SIMD register.
     * @param initial_value The SIMD register.
     */
    force_inline poly_int(simd_type initial_value) noexcept : value(initial_value) { }

    /**
     * @brief Constructs the SIMD register by broadcasting a single integer value.
     * @param initial_value The integer value to broadcast.
     */
    force_inline poly_int(uint32_t initial_value) noexcept {
      value = init(initial_value);
    }

    /**
     * @brief Constructs the SIMD register with four specified integers.
     * @param first  The first  element.
     * @param second The second element.
     * @param third  The third  element.
     * @param fourth The fourth element.
     */
    force_inline poly_int(uint32_t first, uint32_t second, uint32_t third, uint32_t fourth) noexcept {
      scalar_simd_union union_value { (int32_t)first, (int32_t)second, (int32_t)third, (int32_t)fourth };
      value = union_value.simd;
    }

    /**
     * @brief Constructs a 4-element SIMD register by repeating two values (for SSE2/NEON).
     * @param first  The first element.
     * @param second The second element.
     */
    force_inline poly_int(uint32_t first, uint32_t second) noexcept
      : poly_int(first, second, first, second) { }

    /**
     * @brief Destructor.
     */
    force_inline ~poly_int() noexcept { }

    /**
     * @brief Accessor for an element in the SIMD register.
     * @param index The index (0-based) of the element.
     * @return The corresponding scalar value.
     */
    force_inline uint32_t vector_call access(size_t index) const noexcept {
#if VITAL_AVX2
      simd_union union_value { value };
      return union_value.scalar[index];
#elif VITAL_SSE2
      simd_scalar_union union_value { value };
      return union_value.scalar[index];
#elif VITAL_NEON
      return value[index];
#endif
    }

    /**
     * @brief Sets a specific element in the SIMD register.
     * @param index     The index (0-based) of the element.
     * @param new_value The new value to place at that index.
     */
    force_inline void vector_call set(size_t index, uint32_t new_value) noexcept {
#if VITAL_AVX2
      simd_union union_value { value };
      union_value.scalar[index] = new_value;
      value = union_value.simd;
#elif VITAL_SSE2
      simd_scalar_union union_value { value };
      union_value.scalar[index] = (int32_t)new_value;
      value = union_value.simd;
#elif VITAL_NEON
      value[index] = new_value;
#endif
    }

    /**
     * @brief Operator[] overload (read-only).
     * @param index The index to access.
     * @return The scalar value at that index.
     */
    force_inline uint32_t vector_call operator[](size_t index) const noexcept {
      return access(index);
    }

    /// Compound assignment operators using poly_int
    force_inline poly_int& vector_call operator+=(poly_int other) noexcept {
      value = add(value, other.value);
      return *this;
    }
    force_inline poly_int& vector_call operator-=(poly_int other) noexcept {
      value = sub(value, other.value);
      return *this;
    }
    force_inline poly_int& vector_call operator*=(poly_int other) noexcept {
      value = mul(value, other.value);
      return *this;
    }
    force_inline poly_int& vector_call operator&=(poly_int other) noexcept {
      value = bitAnd(value, other.value);
      return *this;
    }
    force_inline poly_int& vector_call operator|=(poly_int other) noexcept {
      value = bitOr(value, other.value);
      return *this;
    }
    force_inline poly_int& vector_call operator^=(poly_int other) noexcept {
      value = bitXor(value, other.value);
      return *this;
    }

    /// Compound assignment operators using simd_type
    force_inline poly_int& vector_call operator+=(simd_type other) noexcept {
      value = add(value, other);
      return *this;
    }
    force_inline poly_int& vector_call operator-=(simd_type other) noexcept {
      value = sub(value, other);
      return *this;
    }
    force_inline poly_int& vector_call operator*=(simd_type other) noexcept {
      value = mul(value, other);
      return *this;
    }
    force_inline poly_int& vector_call operator&=(simd_type other) noexcept {
      value = bitAnd(value, other);
      return *this;
    }
    force_inline poly_int& vector_call operator|=(simd_type other) noexcept {
      value = bitOr(value, other);
      return *this;
    }
    force_inline poly_int& vector_call operator^=(simd_type other) noexcept {
      value = bitXor(value, other);
      return *this;
    }

    /// Compound assignment operators using a scalar
    force_inline poly_int& vector_call operator+=(uint32_t scalar) noexcept {
      value = add(value, init(scalar));
      return *this;
    }
    force_inline poly_int& vector_call operator-=(uint32_t scalar) noexcept {
      value = sub(value, init(scalar));
      return *this;
    }
    force_inline poly_int& vector_call operator*=(uint32_t scalar) noexcept {
      value = mul(value, init(scalar));
      return *this;
    }

    /// Arithmetic operators
    force_inline poly_int vector_call operator+(poly_int other) const noexcept {
      return add(value, other.value);
    }
    force_inline poly_int vector_call operator-(poly_int other) const noexcept {
      return sub(value, other.value);
    }
    force_inline poly_int vector_call operator*(poly_int other) const noexcept {
      return mul(value, other.value);
    }

    /// Bitwise operators
    force_inline poly_int vector_call operator&(poly_int other) const noexcept {
      return bitAnd(value, other.value);
    }
    force_inline poly_int vector_call operator|(poly_int other) const noexcept {
      return bitOr(value, other.value);
    }
    force_inline poly_int vector_call operator^(poly_int other) const noexcept {
      return bitXor(value, other.value);
    }

    /// Unary operators
    force_inline poly_int vector_call operator-() const noexcept {
      return neg(value);
    }
    force_inline poly_int vector_call operator~() const noexcept {
      return bitNot(value);
    }

    /**
     * @brief Sums all elements in the SIMD register.
     * @return Sum of the vector elements.
     */
    force_inline uint32_t vector_call sum() const noexcept {
      return sum(value);
    }

    /**
     * @brief Returns a bitmask for elements that are non-zero.
     * @return Platform-specific bitmask of non-zero elements.
     */
    force_inline uint32_t vector_call anyMask() const noexcept {
      return anyMask(value);
    }
  };

  /// Alias for clarity; used as a mask type in `poly_float`.
  typedef poly_int poly_mask;

  /**
   * @struct poly_float
   * @brief Represents a vector of floating-point values using SIMD instructions.
   *
   * Depending on the available instruction set (AVX2, SSE2, NEON), the vector
   * width differs. This struct provides basic arithmetic, bitwise, and comparison
   * operations on these SIMD vectors.
   */
  struct poly_float {
#if VITAL_AVX2
    static constexpr size_t kSize = 8; ///< Number of elements in the SIMD register
    typedef __m256  simd_type;        ///< Underlying SIMD type for floats
    typedef __m256i mask_simd_type;   ///< Underlying SIMD mask type
#elif VITAL_SSE2
    static constexpr size_t kSize = 4; ///< Number of elements in the SIMD register
    typedef __m128  simd_type;        ///< Underlying SIMD type for floats
    typedef __m128i mask_simd_type;   ///< Underlying SIMD mask type
#elif VITAL_NEON
    static constexpr size_t kSize = 4; ///< Number of elements in the SIMD register
    typedef float32x4_t simd_type;    ///< Underlying SIMD type for floats
    typedef uint32x4_t  mask_simd_type; ///< Underlying SIMD mask type
#endif

    /**
     * @union simd_scalar_union
     * @brief Helper union for copying between a SIMD type and a scalar array.
     */
    union simd_scalar_union {
      simd_type simd;      ///< Access data in SIMD form
      float scalar[kSize]; ///< Access data in scalar form
    };

    /**
     * @union scalar_simd_union
     * @brief Helper union for copying between a scalar array and a SIMD type.
     */
    union scalar_simd_union {
      float scalar[kSize]; ///< Access data in scalar form
      simd_type simd;      ///< Access data in SIMD form
    };

    /**
     * @brief Interprets the bits of a float SIMD register as a mask (integer).
     * @param value The floating-point SIMD register.
     * @return The same bits reinterpreted as a mask_simd_type.
     */
    static force_inline mask_simd_type vector_call toMask(simd_type value) {
#if VITAL_AVX2
      return _mm256_castps_si256(value);
#elif VITAL_SSE2
      return _mm_castps_si128(value);
#elif VITAL_NEON
      return vreinterpretq_u32_f32(value);
#endif
    }

    /**
     * @brief Interprets the bits of a mask SIMD register as float SIMD.
     * @param mask The mask to reinterpret.
     * @return The same bits reinterpreted as a float SIMD register.
     */
    static force_inline simd_type vector_call toSimd(mask_simd_type mask) {
#if VITAL_AVX2
      return _mm256_castsi256_ps(mask);
#elif VITAL_SSE2
      return _mm_castsi128_ps(mask);
#elif VITAL_NEON
      return vreinterpretq_f32_u32(mask);
#endif
    }

    /**
     * @brief Initializes a SIMD register with the same float repeated.
     * @param scalar Float value to broadcast across the SIMD register.
     * @return Initialized SIMD register.
     */
    static force_inline simd_type vector_call init(float scalar) {
#if VITAL_AVX2
      return _mm256_broadcast_ss(&scalar);
#elif VITAL_SSE2
      return _mm_set1_ps(scalar);
#elif VITAL_NEON
      return vdupq_n_f32(scalar);
#endif
    }

    /**
     * @brief Loads floating-point values from memory into a SIMD register.
     * @param memory Pointer to memory location where float data is stored.
     * @return Loaded SIMD register.
     */
    static force_inline simd_type vector_call load(const float* memory) {
#if VITAL_AVX2
      return _mm256_loadu_ps(&scalar);
#elif VITAL_SSE2
      return _mm_loadu_ps(memory);
#elif VITAL_NEON
      return vld1q_f32(memory);
#endif
    }

    /**
     * @brief Adds two SIMD float registers.
     * @param one First operand.
     * @param two Second operand.
     * @return Sum of the two operands.
     */
    static force_inline simd_type vector_call add(simd_type one, simd_type two) {
#if VITAL_AVX2
      return _mm256_add_ps(one, two);
#elif VITAL_SSE2
      return _mm_add_ps(one, two);
#elif VITAL_NEON
      return vaddq_f32(one, two);
#endif
    }

    /**
     * @brief Subtracts one SIMD float register from another.
     * @param one First operand.
     * @param two Second operand.
     * @return Result of `one - two`.
     */
    static force_inline simd_type vector_call sub(simd_type one, simd_type two) {
#if VITAL_AVX2
      return _mm256_sub_ps(one, two);
#elif VITAL_SSE2
      return _mm_sub_ps(one, two);
#elif VITAL_NEON
      return vsubq_f32(one, two);
#endif
    }

    /**
     * @brief Negates a SIMD float register.
     * @param value SIMD register to negate.
     * @return Negation of the input register.
     */
    static force_inline simd_type vector_call neg(simd_type value) {
#if VITAL_AVX2
      return _mm256_xor_ps(value, _mm256_set1_ps(-0.f));
#elif VITAL_SSE2
      return _mm_xor_ps(value, _mm_set1_ps(-0.f));
#elif VITAL_NEON
      return vmulq_n_f32(value, -1.0f);
#endif
    }

    /**
     * @brief Multiplies two SIMD float registers element-wise.
     * @param one First operand.
     * @param two Second operand.
     * @return Product of the two operands.
     */
    static force_inline simd_type vector_call mul(simd_type one, simd_type two) {
#if VITAL_AVX2
      return _mm256_mul_ps(one, two);
#elif VITAL_SSE2
      return _mm_mul_ps(one, two);
#elif VITAL_NEON
      return vmulq_f32(one, two);
#endif
    }

    /**
     * @brief Multiplies a SIMD float register by a float scalar.
     * @param value The SIMD float register.
     * @param scalar The float scalar.
     * @return Product of each element in the register by the scalar.
     */
    static force_inline simd_type vector_call mulScalar(simd_type value, float scalar) {
#if VITAL_AVX2
      return _mm256_mul_ps(value, _mm_set1_ps(scalar));
#elif VITAL_SSE2
      return _mm_mul_ps(value, _mm_set1_ps(scalar));
#elif VITAL_NEON
      return vmulq_n_f32(value, scalar);
#endif
    }

    /**
     * @brief Fused multiply-add operation: `one = one + (two * three)`.
     * @param one   Accumulator register.
     * @param two   First multiplier operand.
     * @param three Second multiplier operand.
     * @return Result of the fused multiply-add.
     */
    static force_inline simd_type vector_call mulAdd(simd_type one, simd_type two, simd_type three) {
#if VITAL_AVX2
      return _mm256_fmadd_ps(two, three, one);
#elif VITAL_SSE2
      return _mm_add_ps(one, _mm_mul_ps(two, three));
#elif VITAL_NEON
#if defined(NEON_VFP_V3)
      return vaddq_f32(one, vmulq_f32(two, three));
#else
      return vmlaq_f32(one, two, three);
#endif
#endif
    }

    /**
     * @brief Fused multiply-sub operation: `one = one - (two * three)`.
     * @param one   Accumulator register.
     * @param two   First multiplier operand.
     * @param three Second multiplier operand.
     * @return Result of the fused multiply-sub.
     */
    static force_inline simd_type vector_call mulSub(simd_type one, simd_type two, simd_type three) {
#if VITAL_AVX2
      // _mm256_fsub_ps is not standard;
      // some compilers offer it via FMA extension but it's not in the snippet.
      // Could emulate: return _mm256_sub_ps(one, _mm256_mul_ps(two, three));
      #error "AVX2 mulSub is not implemented in this snippet"
#elif VITAL_SSE2
      return _mm_sub_ps(one, _mm_mul_ps(two, three));
#elif VITAL_NEON
#if defined(NEON_VFP_V3)
      return vsubq_f32(one, vmulq_f32(two, three));
#else
      return vmlsq_f32(one, two, three);
#endif
#endif
    }

    /**
     * @brief Divides one SIMD float register by another, element-wise.
     * @param one Dividend.
     * @param two Divisor.
     * @return Result of `one / two`.
     */
    static force_inline simd_type vector_call div(simd_type one, simd_type two) {
#if VITAL_AVX2
      return _mm256_div_ps(one, two);
#elif VITAL_SSE2
      return _mm_div_ps(one, two);
#elif VITAL_NEON
#if defined(NEON_ARM32)
      // Approximate reciprocal then refine
      simd_type reciprocal = vrecpeq_f32(two);
      reciprocal = vmulq_f32(vrecpsq_f32(two, reciprocal), reciprocal);
      reciprocal = vmulq_f32(vrecpsq_f32(two, reciprocal), reciprocal);
      return vmulq_f32(one, reciprocal);
#else
      return vdivq_f32(one, two);
#endif
#endif
    }

    /**
     * @brief Bitwise AND of a float SIMD register with a mask.
     * @param value The floating-point operand.
     * @param mask  The integer mask (reinterpreted as float).
     * @return Bitwise AND result.
     */
    static force_inline simd_type vector_call bitAnd(simd_type value, mask_simd_type mask) {
#if VITAL_AVX2
      return _mm256_and_ps(value, toSimd(mask));
#elif VITAL_SSE2
      return _mm_and_ps(value, toSimd(mask));
#elif VITAL_NEON
      return toSimd(vandq_u32(toMask(value), mask));
#endif
    }

    /**
     * @brief Bitwise OR of a float SIMD register with a mask.
     * @param value The floating-point operand.
     * @param mask  The integer mask (reinterpreted as float).
     * @return Bitwise OR result.
     */
    static force_inline simd_type vector_call bitOr(simd_type value, mask_simd_type mask) {
#if VITAL_AVX2
      return _mm256_or_ps(value, toSimd(mask));
#elif VITAL_SSE2
      return _mm_or_ps(value, toSimd(mask));
#elif VITAL_NEON
      return toSimd(vorrq_u32(toMask(value), mask));
#endif
    }

    /**
     * @brief Bitwise XOR of a float SIMD register with a mask.
     * @param value The floating-point operand.
     * @param mask  The integer mask (reinterpreted as float).
     * @return Bitwise XOR result.
     */
    static force_inline simd_type vector_call bitXor(simd_type value, mask_simd_type mask) {
#if VITAL_AVX2
      return _mm256_xor_ps(value, toSimd(mask));
#elif VITAL_SSE2
      return _mm_xor_ps(value, toSimd(mask));
#elif VITAL_NEON
      return toSimd(veorq_u32(toMask(value), mask));
#endif
    }

    /**
     * @brief Bitwise NOT of a float SIMD register.
     * @param value The operand to invert.
     * @return Bitwise NOT of the value.
     */
    static force_inline simd_type vector_call bitNot(simd_type value) {
      return bitXor(value, poly_mask::init(-1));
    }

    /**
     * @brief Returns the element-wise maximum of two SIMD float registers.
     * @param one First operand.
     * @param two Second operand.
     * @return Element-wise maximum.
     */
    static force_inline simd_type vector_call max(simd_type one, simd_type two) {
#if VITAL_AVX2
      return _mm256_max_ps(one, two);
#elif VITAL_SSE2
      return _mm_max_ps(one, two);
#elif VITAL_NEON
      return vmaxq_f32(one, two);
#endif
    }

    /**
     * @brief Returns the element-wise minimum of two SIMD float registers.
     * @param one First operand.
     * @param two Second operand.
     * @return Element-wise minimum.
     */
    static force_inline simd_type vector_call min(simd_type one, simd_type two) {
#if VITAL_AVX2
      return _mm256_min_ps(one, two);
#elif VITAL_SSE2
      return _mm_min_ps(one, two);
#elif VITAL_NEON
      return vminq_f32(one, two);
#endif
    }

    /**
     * @brief Computes the absolute value of each element in the SIMD float register.
     * @param value The floating-point operand.
     * @return The element-wise absolute value.
     */
    static force_inline simd_type vector_call abs(simd_type value) {
      return bitAnd(value, poly_mask::init(poly_mask::kNotSignMask));
    }

    /**
     * @brief Extracts the sign bit mask from each element in the SIMD float register.
     * @param value The floating-point operand.
     * @return A mask where each element has the sign bit of the corresponding float.
     */
    static force_inline mask_simd_type vector_call sign_mask(simd_type value) {
      return toMask(bitAnd(value, poly_mask::init(poly_mask::kSignMask)));
    }

    /**
     * @brief Compares two SIMD float registers for equality, element-wise.
     * @param one First operand.
     * @param two Second operand.
     * @return A mask (all bits set where equal, zero otherwise).
     */
    static force_inline mask_simd_type vector_call equal(simd_type one, simd_type two) {
#if VITAL_AVX2
      // In the snippet, it tries _mm256_cmpeq_ps with a second param, but the real call is just `_mm256_cmp_ps(one, two, _CMP_EQ_OQ)`.
      return toMask(_mm256_cmp_ps(one, two, _CMP_EQ_OQ));
#elif VITAL_SSE2
      return toMask(_mm_cmpeq_ps(one, two));
#elif VITAL_NEON
      return vceqq_f32(one, two);
#endif
    }

    /**
     * @brief Compares two SIMD float registers, element-wise, for greater than.
     * @param one First operand.
     * @param two Second operand.
     * @return A mask (all bits set where one[i] > two[i], zero otherwise).
     */
    static force_inline mask_simd_type vector_call greaterThan(simd_type one, simd_type two) {
#if VITAL_AVX2
      return toMask(_mm256_cmp_ps(one, two, _CMP_GT_OQ));
#elif VITAL_SSE2
      return toMask(_mm_cmpgt_ps(one, two));
#elif VITAL_NEON
      return vcgtq_f32(one, two);
#endif
    }

    /**
     * @brief Compares two SIMD float registers, element-wise, for greater than or equal.
     * @param one First operand.
     * @param two Second operand.
     * @return A mask (all bits set where one[i] >= two[i], zero otherwise).
     */
    static force_inline mask_simd_type vector_call greaterThanOrEqual(simd_type one, simd_type two) {
#if VITAL_AVX2
      return toMask(_mm256_cmp_ps(one, two, _CMP_GE_OQ));
#elif VITAL_SSE2
      return toMask(_mm_cmpge_ps(one, two));
#elif VITAL_NEON
      return vcgeq_f32(one, two);
#endif
    }

    /**
     * @brief Compares two SIMD float registers for non-equality, element-wise.
     * @param one First operand.
     * @param two Second operand.
     * @return A mask (all bits set where one[i] != two[i], zero otherwise).
     */
    static force_inline mask_simd_type vector_call notEqual(simd_type one, simd_type two) {
#if VITAL_AVX2
      return toMask(_mm256_cmp_ps(one, two, _CMP_NEQ_OQ));
#elif VITAL_SSE2
      return toMask(_mm_cmpneq_ps(one, two));
#elif VITAL_NEON
      poly_mask greater = greaterThan(one, two);
      poly_mask less    = lessThan(one, two);
      return poly_mask::bitOr(greater.value, less.value);
#endif
    }

    /**
     * @brief Computes the sum of all elements in a SIMD float register.
     * @param value The SIMD register to sum.
     * @return The sum of all elements.
     */
    static force_inline float vector_call sum(simd_type value) {
#if VITAL_AVX2
      // Example logic (not fully implemented):
      // simd_type flip = _mm256_permute2f128_ps(value, value, 1);
      // ...
      // return ...
      // Implementation incomplete in the snippet.
      #error "AVX2 version not fully implemented in code snippet"
#elif VITAL_SSE2
      simd_type flip = _mm_shuffle_ps(value, value, _MM_SHUFFLE(1, 0, 3, 2));
      simd_type sum_vec = _mm_add_ps(value, flip);
      simd_type swap = _mm_shuffle_ps(sum_vec, sum_vec, _MM_SHUFFLE(2, 3, 0, 1));
      return _mm_cvtss_f32(_mm_add_ps(sum_vec, swap));
#elif VITAL_NEON
      float32x2_t partial_sum = vpadd_f32(vget_low_f32(value), vget_high_f32(value));
      partial_sum = vpadd_f32(partial_sum, partial_sum);
      return vget_lane_f32(partial_sum, 0);
#endif
    }

    /**
     * @brief Performs an in-place 4x4 transpose of four SSE/NEON registers containing float data.
     *
     * @param row0 Row 0 (in/out).
     * @param row1 Row 1 (in/out).
     * @param row2 Row 2 (in/out).
     * @param row3 Row 3 (in/out).
     */
    static force_inline void vector_call transpose(simd_type& row0, simd_type& row1,
                                                   simd_type& row2, simd_type& row3) {
#if VITAL_AVX2
      static_assert(false, "AVX2 transpose not supported yet");
#elif VITAL_SSE2
      __m128 low0 = _mm_unpacklo_ps(row0, row1);
      __m128 low1 = _mm_unpacklo_ps(row2, row3);
      __m128 high0 = _mm_unpackhi_ps(row0, row1);
      __m128 high1 = _mm_unpackhi_ps(row2, row3);
      row0 = _mm_movelh_ps(low0, low1);
      row1 = _mm_movehl_ps(low1, low0);
      row2 = _mm_movelh_ps(high0, high1);
      row3 = _mm_movehl_ps(high1, high0);
#elif VITAL_NEON
      float32x4x2_t swap_low = vtrnq_f32(row0, row1);
      float32x4x2_t swap_high = vtrnq_f32(row2, row3);
      // This snippet attempts to emulate a 4x4 transpose with vtrnq and vextq
      row0 = vextq_f32(vextq_f32(swap_low.val[0], swap_low.val[0], 2), swap_high.val[0], 2);
      row1 = vextq_f32(vextq_f32(swap_low.val[1], swap_low.val[1], 2), swap_high.val[1], 2);
      row2 = vextq_f32(swap_low.val[0], vextq_f32(swap_high.val[0], swap_high.val[0], 2), 2);
      row3 = vextq_f32(swap_low.val[1], vextq_f32(swap_high.val[1], swap_high.val[1], 2), 2);
#else
      // No-op or error for other platforms
#endif
    }

    /// Convenience overloads returning poly_float instead of simd_type:
    static force_inline poly_float vector_call mulAdd(poly_float one, poly_float two, poly_float three) {
      return mulAdd(one.value, two.value, three.value);
    }
    static force_inline poly_float vector_call mulSub(poly_float one, poly_float two, poly_float three) {
      return mulSub(one.value, two.value, three.value);
    }
    static force_inline poly_float vector_call max(poly_float one, poly_float two) {
      return max(one.value, two.value);
    }
    static force_inline poly_float vector_call min(poly_float one, poly_float two) {
      return min(one.value, two.value);
    }
    static force_inline poly_float vector_call abs(poly_float value) {
      return abs(value.value);
    }
    static force_inline poly_mask vector_call sign_mask(poly_float value) {
      return sign_mask(value.value);
    }
    static force_inline poly_mask vector_call equal(poly_float one, poly_float two) {
      return equal(one.value, two.value);
    }
    static force_inline poly_mask vector_call notEqual(poly_float one, poly_float two) {
      return notEqual(one.value, two.value);
    }
    static force_inline poly_mask vector_call greaterThan(poly_float one, poly_float two) {
      return greaterThan(one.value, two.value);
    }
    static force_inline poly_mask vector_call greaterThanOrEqual(poly_float one, poly_float two) {
      return greaterThanOrEqual(one.value, two.value);
    }
    static force_inline poly_mask vector_call lessThan(poly_float one, poly_float two) {
      return greaterThan(two.value, one.value);
    }
    static force_inline poly_mask vector_call lessThanOrEqual(poly_float one, poly_float two) {
      return greaterThanOrEqual(two.value, one.value);
    }

    simd_type value; ///< The underlying SIMD register for float.

    /**
     * @brief Default constructor. Initializes to zero (0.0f).
     */
    force_inline poly_float() noexcept { value = init(0.0f); }

    /**
     * @brief Constructs from a raw SIMD register.
     * @param initial_value The SIMD register.
     */
    force_inline poly_float(simd_type initial_value) noexcept : value(initial_value) { }

    /**
     * @brief Constructs the SIMD register by broadcasting a single float value.
     * @param initial_value The float value to broadcast.
     */
    force_inline poly_float(float initial_value) noexcept {
      value = init(initial_value);
    }

    /**
     * @brief Constructs a SIMD register by repeating two float values (for SSE2/NEON).
     * @param initial_value1 The first element.
     * @param initial_value2 The second element.
     */
    force_inline poly_float(float initial_value1, float initial_value2) noexcept {
      scalar_simd_union union_value { initial_value1, initial_value2, initial_value1, initial_value2 };
      value = union_value.simd;
    }

    /**
     * @brief Constructs a SIMD register with four specified floats (for SSE2/NEON).
     * @param first  The first element.
     * @param second The second element.
     * @param third  The third element.
     * @param fourth The fourth element.
     */
    force_inline poly_float(float first, float second, float third, float fourth) noexcept {
      scalar_simd_union union_value { first, second, third, fourth };
      value = union_value.simd;
    }

    /**
     * @brief Destructor.
     */
    force_inline ~poly_float() noexcept { }

    /**
     * @brief Accessor for an element in the SIMD register.
     * @param index The index (0-based) of the element.
     * @return The corresponding scalar value.
     */
    force_inline float vector_call access(size_t index) const noexcept {
#if VITAL_AVX2
      simd_union union_value { value };
      return union_value.scalar[index];
#elif VITAL_SSE2
      simd_scalar_union union_value { value };
      return union_value.scalar[index];
#elif VITAL_NEON
      return value[index];
#endif
    }

    /**
     * @brief Sets a specific element in the SIMD register.
     * @param index     The index (0-based) of the element.
     * @param new_value The new value to place at that index.
     */
    force_inline void vector_call set(size_t index, float new_value) noexcept {
#if VITAL_AVX2
      simd_union union_value { value };
      union_value.scalar[index] = new_value;
      value = union_value.simd;
#elif VITAL_SSE2
      simd_scalar_union union_value { value };
      union_value.scalar[index] = new_value;
      value = union_value.simd;
#elif VITAL_NEON
      value[index] = new_value;
#endif
    }

    /**
     * @brief Operator[] overload (read-only).
     * @param index The index to access.
     * @return The scalar value at that index.
     */
    force_inline float vector_call operator[](size_t index) const noexcept {
      return access(index);
    }

    /// Compound assignment operators using poly_float
    force_inline poly_float& vector_call operator+=(poly_float other) noexcept {
      value = add(value, other.value);
      return *this;
    }
    force_inline poly_float& vector_call operator-=(poly_float other) noexcept {
      value = sub(value, other.value);
      return *this;
    }
    force_inline poly_float& vector_call operator*=(poly_float other) noexcept {
      value = mul(value, other.value);
      return *this;
    }
    force_inline poly_float& vector_call operator/=(poly_float other) noexcept {
      value = div(value, other.value);
      return *this;
    }
    force_inline poly_float& vector_call operator&=(poly_mask other) noexcept {
      value = bitAnd(value, other.value);
      return *this;
    }
    force_inline poly_float& vector_call operator|=(poly_mask other) noexcept {
      value = bitOr(value, other.value);
      return *this;
    }
    force_inline poly_float& vector_call operator^=(poly_mask other) noexcept {
      value = bitXor(value, other.value);
      return *this;
    }

    /// Compound assignment operators using simd_type
    force_inline poly_float& vector_call operator+=(simd_type other) noexcept {
      value = add(value, other);
      return *this;
    }
    force_inline poly_float& vector_call operator-=(simd_type other) noexcept {
      value = sub(value, other);
      return *this;
    }
    force_inline poly_float& vector_call operator*=(simd_type other) noexcept {
      value = mul(value, other);
      return *this;
    }
    force_inline poly_float& vector_call operator/=(simd_type other) noexcept {
      value = div(value, other);
      return *this;
    }
    force_inline poly_float& vector_call operator&=(mask_simd_type other) noexcept {
      value = bitAnd(value, other);
      return *this;
    }
    force_inline poly_float& vector_call operator|=(mask_simd_type other) noexcept {
      value = bitOr(value, other);
      return *this;
    }
    force_inline poly_float& vector_call operator^=(mask_simd_type other) noexcept {
      value = bitXor(value, other);
      return *this;
    }

    /// Compound assignment operators using a scalar
    force_inline poly_float& vector_call operator+=(float scalar) noexcept {
      value = add(value, init(scalar));
      return *this;
    }
    force_inline poly_float& vector_call operator-=(float scalar) noexcept {
      value = sub(value, init(scalar));
      return *this;
    }
    force_inline poly_float& vector_call operator*=(float scalar) noexcept {
      value = mulScalar(value, scalar);
      return *this;
    }
    force_inline poly_float& vector_call operator/=(float scalar) noexcept {
      value = div(value, init(scalar));
      return *this;
    }

    /// Arithmetic operators
    force_inline poly_float vector_call operator+(poly_float other) const noexcept {
      return add(value, other.value);
    }
    force_inline poly_float vector_call operator-(poly_float other) const noexcept {
      return sub(value, other.value);
    }
    force_inline poly_float vector_call operator*(poly_float other) const noexcept {
      return mul(value, other.value);
    }
    force_inline poly_float vector_call operator/(poly_float other) const noexcept {
      return div(value, other.value);
    }
    force_inline poly_float vector_call operator*(float scalar) const noexcept {
      return mulScalar(value, scalar);
    }
    force_inline poly_float vector_call operator&(poly_mask other) const noexcept {
      return bitAnd(value, other.value);
    }
    force_inline poly_float vector_call operator|(poly_mask other) const noexcept {
      return bitOr(value, other.value);
    }
    force_inline poly_float vector_call operator^(poly_mask other) const noexcept {
      return bitXor(value, other.value);
    }

    /// Unary operators
    force_inline poly_float vector_call operator-() const noexcept {
      return neg(value);
    }
    force_inline poly_float vector_call operator~() const noexcept {
      return bitNot(value);
    }

    /**
     * @brief Sums all elements in the SIMD float register.
     * @return Sum of the vector elements.
     */
    force_inline float vector_call sum() const noexcept {
      return sum(value);
    }
  };

} // namespace vital
