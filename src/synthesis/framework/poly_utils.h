#pragma once

#include "matrix.h"
#include "utils.h"

#include <cmath>

namespace vital {
  /**
   * @namespace vital::utils
   * @brief A collection of inline helper functions and constants used for
   *        SIMD computations, interpolation, and other audio-related utilities.
   *
   * The functions within this namespace primarily operate on `poly_float` and `poly_int`
   * (SIMD vector types) to efficiently handle multi-sample or multi-voice operations.
   */
  namespace utils {
    /**
     * @brief Used as a multiplier when encoding phase and voice data in a single float.
     */
    constexpr mono_float kPhaseEncodingMultiplier = 0.9f;

    /**
     * @brief A bitmask used in voice encoding for note-on states.
     */
    constexpr unsigned int kNotePressedMask = 0xf;

    /**
     * @brief A static `poly_float` used to split or combine stereo channels.
     *        Typically has values (1.0, -1.0).
     */
    static const poly_float kStereoSplit(1.0f, -1.0f);

    /**
     * @brief Predefined constants for Lagrange and polynomial interpolation.
     */
    static const poly_float kLagrangeOne(0.0f, 1.0f, 0.0f, 0.0f);
    static const poly_float kLagrangeTwo(-1.0f, -1.0f, 1.0f, 1.0f);
    static const poly_float kLagrangeThree(-2.0f, -2.0f, -2.0f, -1.0f);
    static const poly_float kLagrangeMult(-1.0f / 6.0f, 1.0f / 2.0f, -1.0f / 2.0f, 1.0f / 6.0f);

    /**
     * @brief Optimal interpolation constants used in some polynomial approximations.
     */
    static const poly_float kOptimalOne(0.00224072707074864375f, 0.20184198969656244725f,
                                        0.59244492420272312725f, 0.20345744715566445625f);
    static const poly_float kOptimalTwo(-0.0059513775678254975f, -0.456633315206820491f,
                                        -0.035736698832993691f, 0.4982319203618311775f);
    static const poly_float kOptimalThree(0.093515484757265265f, 0.294278871937834749f,
                                          -0.786648885977648931f, 0.398765058036740415f);
    static const poly_float kOptimalFour(-0.10174985775982505f, 0.36030925263849456f,
                                         -0.36030925263849456f, 0.10174985775982505f);

    /**
     * @brief Performs a fused multiply-add on SIMD data: (a * b) + c.
     * @param a The first multiplicand.
     * @param b The second multiplicand.
     * @param c The addend.
     * @return Result of `(a * b) + c`.
     */
    force_inline poly_float mulAdd(poly_float a, poly_float b, poly_float c) {
      return poly_float::mulAdd(a, b, c);
    }

    /**
     * @brief Performs a fused multiply-sub on SIMD data: (a * b) - c.
     * @param a The first multiplicand.
     * @param b The second multiplicand.
     * @param c The subtrahend.
     * @return Result of `(a * b) - c`.
     */
    force_inline poly_float mulSub(poly_float a, poly_float b, poly_float c) {
      return poly_float::mulSub(a, b, c);
    }

    /**
     * @brief Applies a scalar function to each element in a `poly_float`.
     * @tparam func Pointer to a function (mono_float -> mono_float).
     * @param value The SIMD vector on which to apply the function element-by-element.
     * @return A new SIMD vector containing `func(value[i])` for each lane.
     */
    template<mono_float(*func)(mono_float)>
    force_inline poly_float map(poly_float value) {
      poly_float result;
      for (int i = 0; i < poly_float::kSize; ++i)
        result.set(i, func(value[i]));
      return result;
    }

    /**
     * @brief Converts semitone cents to a linear frequency ratio (vectorized).
     * @param value The input in cents (e.g., 1200 = +1 octave).
     * @return The ratio as `2^(value / 1200)`.
     */
    force_inline poly_float centsToRatio(poly_float value) {
      return map<centsToRatio>(value);
    }

    /**
     * @brief Converts note offsets to frequency ratios. Similar to `centsToRatio`, but may differ in how offset is defined.
     */
    force_inline poly_float noteOffsetToRatio(poly_float value) {
      return map<noteOffsetToRatio>(value);
    }

    /**
     * @brief Converts a ratio to MIDI transpose (vectorized).
     */
    force_inline poly_float ratioToMidiTranspose(poly_float value) {
      return map<ratioToMidiTranspose>(value);
    }

    /**
     * @brief Converts MIDI cents to frequency (vectorized).
     */
    force_inline poly_float midiCentsToFrequency(poly_float value) {
      return map<midiCentsToFrequency>(value);
    }

    /**
     * @brief Converts a MIDI note to a frequency (vectorized).
     */
    force_inline poly_float midiNoteToFrequency(poly_float value) {
      return map<midiNoteToFrequency>(value);
    }

    /**
     * @brief Converts a frequency to a MIDI note (vectorized).
     */
    force_inline poly_float frequencyToMidiNote(poly_float value) {
      return map<frequencyToMidiNote>(value);
    }

    /**
     * @brief Converts a frequency to MIDI cents (vectorized).
     */
    force_inline poly_float frequencyToMidiCents(poly_float value) {
      return map<frequencyToMidiCents>(value);
    }

    /**
     * @brief Converts a magnitude value to decibels (vectorized).
     */
    force_inline poly_float magnitudeToDb(poly_float value) { return map<magnitudeToDb>(value); }

    /**
     * @brief Converts a dB value to linear magnitude (vectorized).
     */
    force_inline poly_float dbToMagnitude(poly_float value) { return map<dbToMagnitude>(value); }

    /**
     * @brief Computes the tangent of each element (in radians) in a `poly_float`.
     */
    force_inline poly_float tan(poly_float value) { return map<tanf>(value); }

    /**
     * @brief Computes the sine of each element (in radians).
     */
    force_inline poly_float sin(poly_float value) { return map<sinf>(value); }

    /**
     * @brief Computes the cosine of each element (in radians).
     */
    force_inline poly_float cos(poly_float value) { return map<cosf>(value); }

    /**
     * @brief Computes the square root of each element in a `poly_float`.
     */
    force_inline poly_float sqrt(poly_float value) {
    #if VITAL_AVX2
      return _mm256_sqrt_ps(value.value);
    #elif VITAL_SSE2
      return _mm_sqrt_ps(value.value);
    #elif VITAL_NEON
      return map<sqrtf>(value);
    #endif
    }

    /**
     * @brief Performs a linear interpolation between two `poly_float`s using a scalar `t` in [0..1].
     */
    force_inline poly_float interpolate(poly_float from, poly_float to, mono_float t) {
      return mulAdd(from, to - from, t);
    }

    /**
     * @brief Returns the cubic Lagrange interpolation constants for a scalar `mono_t`.
     */
    force_inline poly_float getCubicInterpolationValues(mono_float mono_t) {
      poly_float t = mono_t;
      return kLagrangeMult * (t + kLagrangeOne) * (t + kLagrangeTwo) * (t + kLagrangeThree);
    }

    /**
     * @brief Returns an "optimal" polynomial interpolation for a scalar `mono_t`.
     */
    force_inline poly_float getOptimalInterpolationValues(mono_float mono_t) {
      poly_float t = mono_t;
      return ((kOptimalFour * t + kOptimalThree) * t + kOptimalTwo) * t + kOptimalOne;
    }

    /**
     * @brief Creates a matrix for polynomial interpolation given a starting `poly_float` t_from.
     */
    force_inline matrix getPolynomialInterpolationMatrix(poly_float t_from) {
      static constexpr mono_float kMultPrev = -1.0f / 6.0f;
      static constexpr mono_float kMultFrom = 1.0f / 2.0f;
      static constexpr mono_float kMultTo = -1.0f / 2.0f;
      static constexpr mono_float kMultNext = 1.0f / 6.0f;

      poly_float t_prev = t_from + 1.0f;
      poly_float t_to = t_from - 1.0f;
      poly_float t_next = t_from - 2.0f;

      poly_float t_prev_from = t_prev * t_from;
      poly_float t_to_next = t_to * t_next;

      return matrix(t_from * t_to_next * kMultPrev,
                    t_prev * t_to_next * kMultFrom,
                    t_prev_from * t_next * kMultTo,
                    t_prev_from * t_to * kMultNext);
    }

    /**
     * @brief Creates a Catmull-Rom interpolation matrix from a `poly_float` t.
     */
    force_inline matrix getCatmullInterpolationMatrix(poly_float t) {
      poly_float half_t = t * 0.5f;
      poly_float half_t2 = t * half_t;
      poly_float half_t3 = half_t2 * t;
      poly_float half_three_t3 = half_t3 * 3.0f;

      return matrix(half_t2 * 2.0f - half_t3 - half_t,
                    mulSub(half_three_t3, half_t2, 5.0f) + 1.0f,
                    mulAdd(half_t, half_t2, 4.0f) - half_three_t3,
                    half_t3 - half_t2);
    }

    /**
     * @brief Creates a matrix for simple linear interpolation using scalar or vector `t`.
     */
    force_inline matrix getLinearInterpolationMatrix(poly_float t) {
      return matrix(0.0f, poly_float(1.0f) - t, t, 0.0f);
    }

    /**
     * @brief Loads a `poly_float` from an unaligned float pointer.
     */
    force_inline poly_float toPolyFloatFromUnaligned(const mono_float* unaligned) {
    #if VITAL_AVX2
      return _mm256_loadu_ps(unaligned);
    #elif VITAL_SSE2
      return _mm_loadu_ps(unaligned);
    #elif VITAL_NEON
      return vld1q_f32(unaligned);
    #endif
    }

    /**
     * @brief Creates a matrix of 4 poly_float lanes from a single buffer at varying indices.
     */
    force_inline matrix getValueMatrix(const mono_float* buffer, poly_int indices) {
      return matrix(toPolyFloatFromUnaligned(buffer + indices[0]),
                    toPolyFloatFromUnaligned(buffer + indices[1]),
                    toPolyFloatFromUnaligned(buffer + indices[2]),
                    toPolyFloatFromUnaligned(buffer + indices[3]));
    }

    /**
     * @brief Creates a matrix of 4 poly_float lanes from 4 separate buffers at varying indices.
     */
    force_inline matrix getValueMatrix(const mono_float* const* buffers, poly_int indices) {
      return matrix(toPolyFloatFromUnaligned(buffers[0] + indices[0]),
                    toPolyFloatFromUnaligned(buffers[1] + indices[1]),
                    toPolyFloatFromUnaligned(buffers[2] + indices[2]),
                    toPolyFloatFromUnaligned(buffers[3] + indices[3]));
    }

    /**
     * @brief Interpolates between two SIMD values with a SIMD fraction t.
     */
    force_inline poly_float interpolate(poly_float from, poly_float to, poly_float t) {
      return mulAdd(from, to - from, t);
    }

    /**
     * @brief Interpolates between two scalars with a SIMD fraction t.
     */
    force_inline poly_float interpolate(mono_float from, mono_float to, poly_float t) {
      return mulAdd(from, to - from, t);
    }

    /**
     * @brief A specialized interpolation function used in perlin-like routines.
     */
    force_inline poly_float perlinInterpolate(poly_float from, poly_float to, poly_float t) {
      poly_float interpolate_from = from * t;
      poly_float interpolate_to = to * (t - 1.0f);
      poly_float interpolate_t = t * t * (t * -2.0f + 3.0f);
      return interpolate(interpolate_from, interpolate_to, interpolate_t) * 2.0f;
    }

    /**
     * @brief Clamps each lane of a vector to [min, max].
     */
    force_inline poly_float clamp(poly_float value, mono_float min, mono_float max) {
      return poly_float::max(poly_float::min(value, max), min);
    }

    /**
     * @brief Clamps each lane of a vector to [min, max], where min and max are themselves `poly_float`s.
     */
    force_inline poly_float clamp(poly_float value, poly_float min, poly_float max) {
      return poly_float::max(poly_float::min(value, max), min);
    }

    /**
     * @brief Clamps a `poly_int` to [min, max].
     */
    force_inline poly_int clamp(poly_int value, poly_int min, poly_int max) {
      return poly_int::max(poly_int::min(value, max), min);
    }

    /**
     * @brief Returns the maximum of two `poly_float`s lane-by-lane.
     */
    force_inline poly_float max(poly_float left, poly_float right) {
      return poly_float::max(left, right);
    }

    /**
     * @brief Returns the minimum of two `poly_float`s lane-by-lane.
     */
    force_inline poly_float min(poly_float left, poly_float right) {
      return poly_float::min(left, right);
    }

    /**
     * @brief Checks if two `poly_float`s are equal lane-by-lane. Returns true if all lanes match.
     */
    force_inline bool equal(poly_float left, poly_float right) {
      return poly_float::notEqual(left, right).sum() == 0;
    }

    /**
     * @brief Selects between two values (zero_value or one_value) based on a mask in each lane.
     * @param zero_value Value to select for false lanes.
     * @param one_value Value to select for true lanes.
     * @param reset_mask The mask indicating which lanes are true/false.
     */
    force_inline poly_float maskLoad(poly_float zero_value, poly_float one_value, poly_mask reset_mask) {
      poly_float old_values = zero_value & ~reset_mask;
      poly_float new_values = one_value & reset_mask;

      return old_values + new_values;
    }

    /**
     * @brief Overload for `poly_int`.
     */
    force_inline poly_int maskLoad(poly_int zero_value, poly_int one_value, poly_mask reset_mask) {
      poly_int old_values = zero_value & ~reset_mask;
      poly_int new_values = one_value & reset_mask;

      return old_values | new_values;
    }

    /**
     * @brief A special mod operation that ensures the result stays below 1.0, subtracting 1.0 if needed.
     * @param value The input vector in [0..2).
     */
    force_inline poly_float modOnce(poly_float value) {
      poly_mask less_mask = poly_float::lessThan(value, 1.0f);
      poly_float lower = value - 1.0f;
      return maskLoad(lower, value, less_mask);
    }

    /**
     * @brief Returns a mask where lanes are true if `value` is close to 0.
     */
    force_inline poly_mask closeToZeroMask(poly_float value) {
      return poly_float::lessThan(poly_float::abs(value), kEpsilon);
    }

    /**
     * @brief Raises each lane in `base` to the power of the corresponding lane in `exponent`.
     */
    force_inline poly_float pow(poly_float base, poly_float exponent) {
      poly_float result;
      int size = poly_float::kSize;
      for (int i = 0; i < size; ++i)
        result.set(i, powf(base[i], exponent[i]));
      return result;
    }

    /**
     * @brief Creates a mask indicating whether all values in the given buffer are near zero.
     */
    force_inline poly_mask getSilentMask(const poly_float* buffer, int length) {
      poly_mask silent_mask = poly_float::equal(0.0f, 0.0f);
      for (int i = 0; i < length; ++i) {
        poly_mask zero_mask = closeToZeroMask(buffer[i]);
        silent_mask &= zero_mask;
      }
      return silent_mask;
    }

    /**
     * @brief Swaps the left and right channels of a stereo `poly_float`.
     */
    force_inline poly_float swapStereo(poly_float value) {
    #if VITAL_AVX2
      return _mm256_shuffle_ps(value.value, value.value, _MM_SHUFFLE(2, 3, 0, 1));
    #elif VITAL_SSE2
      return _mm_shuffle_ps(value.value, value.value, _MM_SHUFFLE(2, 3, 0, 1));
    #elif VITAL_NEON
      return vrev64q_f32(value.value);
    #endif
    }

    /**
     * @brief Same as swapStereo but for `poly_int`.
     */
    force_inline poly_int swapStereo(poly_int value) {
    #if VITAL_AVX2
      return _mm256_shuffle_epi32(value.value, _MM_SHUFFLE(2, 3, 0, 1));
    #elif VITAL_SSE2
      return _mm_shuffle_epi32(value.value, _MM_SHUFFLE(2, 3, 0, 1));
    #elif VITAL_NEON
      return vrev64q_u32(value.value);
    #endif
    }

    /**
     * @brief Swaps the first half of the lanes with the second half.
     */
    force_inline poly_float swapVoices(poly_float value) {
    #if VITAL_AVX2
      return _mm256_shuffle_ps(value.value, value.value, _MM_SHUFFLE(1, 0, 3, 2));
    #elif VITAL_SSE2
      return _mm_shuffle_ps(value.value, value.value, _MM_SHUFFLE(1, 0, 3, 2));
    #elif VITAL_NEON
      return vextq_f32(value.value, value.value, 2);
    #endif
    }

    /**
     * @brief Overload for `poly_int`.
     */
    force_inline poly_int swapVoices(poly_int value) {
    #if VITAL_AVX2
      return _mm256_shuffle_epi32(value.value, value.value, _MM_SHUFFLE(1, 0, 3, 2));
    #elif VITAL_SSE2
      return _mm_shuffle_epi32(value.value, _MM_SHUFFLE(1, 0, 3, 2));
    #elif VITAL_NEON
      return vextq_u32(value.value, value.value, 2);
    #endif
    }

    /**
     * @brief Reorders internal stereo lanes.
     */
    force_inline poly_float swapInner(poly_float value) {
    #if VITAL_AVX2
      return _mm256_shuffle_ps(value.value, value.value, _MM_SHUFFLE(3, 1, 2, 0));
    #elif VITAL_SSE2
      return _mm_shuffle_ps(value.value, value.value, _MM_SHUFFLE(3, 1, 2, 0));
    #elif VITAL_NEON
      float32x4_t rotated = vextq_f32(value.value, value.value, 2);
      float32x4x2_t zipped = vzipq_f32(value.value, rotated);
      return zipped.val[0];
    #endif
    }

    /**
     * @brief Reverses the order of stereo lanes from (L, R, L, R) to (R, L, R, L).
     */
    force_inline poly_float reverse(poly_float value) {
    #if VITAL_AVX2
      return _mm256_shuffle_ps(value.value, value.value, _MM_SHUFFLE(0, 1, 2, 3));
    #elif VITAL_SSE2
      return _mm_shuffle_ps(value.value, value.value, _MM_SHUFFLE(0, 1, 2, 3));
    #elif VITAL_NEON
      return swapVoices(swapStereo(value));
    #endif
    }

    /**
     * @brief Interleaves two stereo `poly_float`s into a single vector with left channels first, then right channels.
     */
    force_inline poly_float consolidateAudio(poly_float one, poly_float two) {
    #if VITAL_AVX2
      return _mm256_unpacklo_ps(one.value, two.value);
    #elif VITAL_SSE2
      return _mm_unpacklo_ps(one.value, two.value);
    #elif VITAL_NEON
      return vzipq_f32(one.value, two.value).val[0];
    #endif
    }

    /**
     * @brief Packs the first voice from two different `poly_float`s into a single vector.
     */
    force_inline poly_float compactFirstVoices(poly_float one, poly_float two) {
    #if VITAL_AVX2
      return _mm256_shuffle_ps(one.value, two.value, _MM_SHUFFLE(1, 0, 1, 0));
    #elif VITAL_SSE2
      return _mm_shuffle_ps(one.value, two.value, _MM_SHUFFLE(1, 0, 1, 0));
    #elif VITAL_NEON
      return vcombine_f32(vget_low_f32(one.value), vget_low_f32(two.value));
    #endif
    }

    /**
     * @brief Adds two stereo lanes for each voice, returning a combined mono result in each lane.
     */
    force_inline poly_float sumSplitAudio(poly_float sum) {
      poly_float totals = sum + utils::swapStereo(sum);
      return utils::swapInner(totals);
    }

    /**
     * @brief Returns the maximum lane value from a `poly_float`.
     */
    force_inline mono_float maxFloat(poly_float values) {
      poly_float swap_voices = swapVoices(values);
      poly_float max_voice = utils::max(values, swap_voices);
      return utils::max(max_voice, utils::swapStereo(max_voice))[0];
    }

    /**
     * @brief Returns the minimum lane value from a `poly_float`.
     */
    force_inline mono_float minFloat(poly_float values) {
      poly_float swap_voices = swapVoices(values);
      poly_float min_voice = utils::min(values, swap_voices);
      return utils::min(min_voice, utils::swapStereo(min_voice))[0];
    }

    /**
     * @brief Converts an L/R stereo representation into M/S (mid/side) encoding.
     */
    force_inline poly_float encodeMidSide(poly_float value) {
      return (value + kStereoSplit * swapStereo(value)) * 0.5f;
    }

    /**
     * @brief Converts a mid/side encoded stereo value back to L/R (decodes).
     */
    force_inline poly_float decodeMidSide(poly_float value) {
      return (value + swapStereo(kStereoSplit * value));
    }

    /**
     * @brief Returns the peak magnitude of a buffer (considering both positive and negative values).
     */
    force_inline poly_float peak(const poly_float* buffer, int num, int skip = 1) {
      poly_float peak = 0.0f;
      for (int i = 0; i < num; i += skip) {
        peak = poly_float::max(peak, buffer[i]);
        peak = poly_float::max(peak, -buffer[i]);
      }

      return peak;
    }

    /**
     * @brief Zeros a mono buffer (standard array).
     */
    force_inline void zeroBuffer(mono_float* buffer, int size) {
      for (int i = 0; i < size; ++i)
        buffer[i] = 0.0f;
    }

    /**
     * @brief Zeros a vector buffer (`poly_float`).
     */
    force_inline void zeroBuffer(poly_float* buffer, int size) {
      for (int i = 0; i < size; ++i)
        buffer[i] = 0.0f;
    }

    /**
     * @brief Copies data from a source mono buffer to a destination mono buffer.
     */
    force_inline void copyBuffer(mono_float* dest, const mono_float* source, int size) {
      for (int i = 0; i < size; ++i)
        dest[i] = source[i];
    }

    /**
     * @brief Copies data from a `poly_float` source buffer to another `poly_float` buffer.
     */
    force_inline void copyBuffer(poly_float* dest, const poly_float* source, int size) {
      for (int i = 0; i < size; ++i)
        dest[i] = source[i];
    }

    /**
     * @brief Adds two `poly_float` buffers element-by-element, storing in `dest`.
     */
    force_inline void addBuffers(poly_float* dest, const poly_float* b1, const poly_float* b2, int size) {
      for (int i = 0; i < size; ++i)
        dest[i] = b1[i] + b2[i];
    }

    /**
     * @brief Checks if all lanes in a `poly_float` are finite.
     */
    force_inline bool isFinite(poly_float value) {
      for (int i = 0; i < poly_float::kSize; ++i) {
        mono_float val = value[i];
        if (!std::isfinite(val))
          return false;
      }
      return true;
    }

    /**
     * @brief Checks if each lane in a `poly_float` is within [min, max].
     */
    force_inline bool isInRange(poly_float value, mono_float min, mono_float max) {
      poly_mask greater_mask = poly_float::greaterThan(value, max);
      poly_mask less_than_mask = poly_float::greaterThan(min, value);
      return (greater_mask.sum() + less_than_mask.sum()) == 0;
    }

    /**
     * @brief Checks if all lanes in a `poly_float` are within a broad range [-8000..8000].
     */
    force_inline bool isContained(poly_float value) {
      static constexpr mono_float kRange = 8000.0f;
      return isInRange(value, -kRange, kRange);
    }

    /**
     * @brief Checks if all values in a buffer of `poly_float`s are finite.
     */
    force_inline bool isFinite(const poly_float* buffer, int size) {
      for (int i = 0; i < size; ++i) {
        if (!isFinite(buffer[i]))
          return false;
      }
      return true;
    }

    /**
     * @brief Checks if all values in a `poly_float` buffer are within [min, max].
     */
    force_inline bool isInRange(const poly_float* buffer, int size, mono_float min, mono_float max) {
      for (int i = 0; i < size; ++i) {
        if (!isInRange(buffer[i], min, max))
          return false;
      }
      return true;
    }

    /**
     * @brief Checks if a buffer of `poly_float` is entirely within [-8000..8000].
     */
    force_inline bool isContained(const poly_float* buffer, int size) {
      static constexpr mono_float kRange = 8000.0f;
      return isInRange(buffer, size, -kRange, kRange);
    }

    /**
     * @brief Determines if the entire buffer is silent (very close to zero).
     */
    force_inline bool isSilent(const poly_float* buffer, int size) {
      const mono_float* mono_buffer = reinterpret_cast<const mono_float*>(buffer);
      return isSilent(mono_buffer, size * poly_float::kSize);
    }

    /**
     * @brief Gathers values from a mono float buffer into a `poly_float`, using per-lane indices.
     */
    force_inline poly_float gather(const mono_float* buffer, const poly_int& indices) {
      poly_float result;
      for (int i = 0; i < poly_float::kSize; ++i) {
        int index = indices[i];
        result.set(i, buffer[index]);
      }
      return result;
    }

    /**
     * @brief Gathers adjacent values for each lane, storing them in `value` and `next`.
     */
    force_inline void adjacentGather(const mono_float* buffer, const poly_int& indices,
                                     poly_float& value, poly_float& next) {
      for (int i = 0; i < poly_float::kSize; ++i) {
        int index = indices[i];
        value.set(i, buffer[index]);
        next.set(i, buffer[index + 1]);
      }
    }

    /**
     * @brief Gathers values from different buffers, with each lane potentially reading from a different buffer index.
     */
    force_inline poly_float gatherSeparate(const mono_float* const* buffers, const poly_int& indices) {
      poly_float result;
      for (int i = 0; i < poly_float::kSize; ++i) {
        int index = indices[i];
        result.set(i, buffers[i][index]);
      }
      return result;
    }

    /**
     * @brief Similar to adjacentGather but for separate buffers.
     */
    force_inline void adjacentGatherSeparate(const mono_float* const* buffers,
                                             const poly_int& indices,
                                             poly_float& value, poly_float& next) {
      for (int i = 0; i < poly_float::kSize; ++i) {
        int index = indices[i];
        value.set(i, buffers[i][index]);
        next.set(i, buffers[i][index + 1]);
      }
    }

    /**
     * @brief Performs a simple filter scaling operation `(power * value) / ((power - 1)*value + 1)`.
     */
    force_inline poly_float fltScale(poly_float value, poly_float power) {
      return power * value / ((power - 1.0f) * value + 1.0f);
    }

    /**
     * @brief Casts a `poly_int` to `poly_float` lane-by-lane.
     */
    force_inline poly_float toFloat(poly_int integers) {
      VITAL_ASSERT(poly_float::kSize == poly_int::kSize);

    #if VITAL_AVX2
      return _mm256_cvtepi32_ps(integers.value);
    #elif VITAL_SSE2
      return _mm_cvtepi32_ps(integers.value);
    #elif VITAL_NEON
      return vcvtq_f32_s32(vreinterpretq_s32_u32(integers.value));
    #endif
    }

    /**
     * @brief Casts a `poly_float` to `poly_int` by truncation.
     */
    force_inline poly_int toInt(poly_float floats) {
      VITAL_ASSERT(poly_float::kSize == poly_int::kSize);

    #if VITAL_AVX2
      return _mm256_cvtps_epi32(floats.value);
    #elif VITAL_SSE2
      return _mm_cvtps_epi32(floats.value);
    #elif VITAL_NEON
      return vreinterpretq_u32_s32(vcvtq_s32_f32(floats.value));
    #endif
    }

    /**
     * @brief Truncates a `poly_float` to an integer (effectively floor for positive values).
     */
    force_inline poly_int truncToInt(poly_float value) {
      return toInt(value);
    }

    /**
     * @brief Returns the truncated value of each lane in `value`.
     */
    force_inline poly_float trunc(poly_float value) {
      return toFloat(truncToInt(value));
    }

    /**
     * @brief Floors each lane in `value`.
     */
    force_inline poly_float floor(poly_float value) {
      poly_float truncated = trunc(value);
      return truncated + (poly_float(-1.0f) & poly_float::greaterThan(truncated, value));
    }

    /**
     * @brief Floors each lane and returns the result as an integer.
     */
    force_inline poly_int floorToInt(poly_float value) {
      return toInt(floor(value));
    }

    /**
     * @brief Rounds each lane to the nearest integer.
     */
    force_inline poly_int roundToInt(poly_float value) {
      return floorToInt(value + 0.5f);
    }

    /**
     * @brief Ceils each lane in `value`.
     */
    force_inline poly_float ceil(poly_float value) {
      poly_float truncated = trunc(value);
      return truncated + (poly_float(1.0f) & poly_float::lessThan(truncated, value));
    }

    /**
     * @brief Rounds each lane to the nearest integer as a `poly_float`.
     */
    force_inline poly_float round(poly_float value) {
      return floor(value + 0.5f);
    }

    /**
     * @brief Returns the fractional part of each lane by subtracting the floored value.
     */
    force_inline poly_float mod(poly_float value) {
      return value - floor(value);
    }

    /**
     * @brief Reinterprets a `poly_int` as a `poly_float` (bitcast).
     */
    force_inline poly_float reinterpretToFloat(poly_int value) {
    #if VITAL_AVX2
      return _mm256_castsi256_ps(value.value);
    #elif VITAL_SSE2
      return _mm_castsi128_ps(value.value);
    #elif VITAL_NEON
      return vreinterpretq_f32_u32(value.value);
    #endif
    }

    /**
     * @brief Reinterprets a `poly_float` as a `poly_int` (bitcast).
     */
    force_inline poly_int reinterpretToInt(poly_float value) {
    #if VITAL_AVX2
      return _mm256_castps_si256(value.value);
    #elif VITAL_SSE2
      return _mm_castps_si128(value.value);
    #elif VITAL_NEON
      return vreinterpretq_u32_f32(value.value);
    #endif
    }

    template<size_t shift>
    force_inline poly_int shiftRight(poly_int integer) {
    #if VITAL_AVX2
      return _mm256_srli_epi32(integers.value, shift);
    #elif VITAL_SSE2
      return _mm_srli_epi32(integer.value, shift);
    #elif VITAL_NEON
      return vshrq_n_u32(integer.value, shift);
    #endif
    }

    template<size_t shift>
    force_inline poly_int shiftLeft(poly_int integer) {
    #if VITAL_AVX2
      return _mm256_slli_epi32(integers.value, shift);
    #elif VITAL_SSE2
      return _mm_slli_epi32(integer.value, shift);
    #elif VITAL_NEON
      return vshlq_n_u32(integer.value, shift);
    #endif
    }

    force_inline poly_float pow2ToFloat(poly_int value) {
      return reinterpretToFloat(shiftLeft<23>(value + 127));
    }

    /**
     * @brief Generates a simple triangle wave [0..1] from a fraction `t` in [0..1].
     */
    force_inline poly_float triangleWave(poly_float t) {
      poly_float adjust = t + 0.75f;
      poly_float range = utils::mod(adjust);
      return poly_float::abs(mulAdd(-1.0f, range, 2.0f));
    }

    /**
     * @brief Computes a cycle offset given a time in seconds and a frequency.
     * @param seconds The elapsed time in seconds.
     * @param frequency The per-lane frequency.
     * @return The fractional cycle offset in [0..1).
     */
    force_inline poly_float getCycleOffsetFromSeconds(double seconds, poly_float frequency) {
      poly_float offset;
      for (int i = 0; i < poly_float::kSize; ++i) {
        double freq = frequency[i];
        double cycles = freq * seconds;
        offset.set(i, cycles - ::floor(cycles));
      }
      return offset;
    }

    /**
     * @brief Computes a cycle offset given a sample count, frequency, sample rate, and oversampling factor.
     */
    force_inline poly_float getCycleOffsetFromSamples(long long samples, poly_float frequency,
                                                      int sample_rate, int oversample_amount) {
      double tick_time = (1.0 * oversample_amount) / sample_rate;
      double seconds_passed = tick_time * samples;
      return getCycleOffsetFromSeconds(seconds_passed, frequency);
    }

    /**
     * @brief Snaps a MIDI transpose value to a quantization mask (e.g., scale degrees).
     */
    force_inline poly_float snapTranspose(poly_float transpose, int quantize) {
      poly_float octave_floored = utils::floor(transpose * (1.0f / kNotesPerOctave)) * kNotesPerOctave;
      poly_float tranpose_from_octave = transpose - octave_floored;
      poly_float min_distance = kNotesPerOctave;
      poly_float transpose_in_octave = tranpose_from_octave;
      for (int i = 0; i <= kNotesPerOctave; ++i) {
        if ((quantize >> (i % kNotesPerOctave)) & 1) {
          poly_float distance = poly_float::abs(tranpose_from_octave - i);
          poly_mask best_mask = poly_float::lessThan(distance, min_distance);
          min_distance = utils::maskLoad(min_distance, distance, best_mask);
          transpose_in_octave = utils::maskLoad(transpose_in_octave, i, best_mask);
        }
      }

      return octave_floored + transpose_in_octave;
    }

    /**
     * @brief Fills a buffer with snap offsets for a given quantize mask.
     */
    force_inline void fillSnapBuffer(int transpose_quantize, float* snap_buffer) {
      float min_snap = 0.0f;
      float max_snap = 0.0f;
      for (int i = 0; i < kNotesPerOctave; ++i) {
        if ((transpose_quantize >> i) & 1) {
          max_snap = i;
          if (min_snap == 0.0f)
            min_snap = i;
        }
      }

      float offset = kNotesPerOctave - max_snap;
      for (int i = 0; i <= kNotesPerOctave; ++i) {
        if ((transpose_quantize >> (i % kNotesPerOctave)) & 1)
          offset = 0.0f;

        snap_buffer[i] = offset;
        offset += 1.0f;
      }
      offset = min_snap;
      for (int i = kNotesPerOctave; i >= 0; --i) {
        if (offset < snap_buffer[i])
          snap_buffer[i] = i + offset;
        else if (snap_buffer[i] != 0.0f)
          snap_buffer[i] = i - snap_buffer[i];
        else {
          snap_buffer[i] = i;
          offset = 0.0f;
        }
        offset += 1.0f;
      }
    }

    /**
     * @brief Checks if the transpose quantize mask applies globally (over multiple octaves).
     */
    force_inline bool isTransposeQuantizeGlobal(int quantize) {
      return quantize >> kNotesPerOctave;
    }

    /**
     * @brief Checks if any snapping bits are set in the transpose quantize mask.
     */
    force_inline bool isTransposeSnapping(int quantize) {
      static constexpr int kTransposeMask = (1 << kNotesPerOctave) - 1;
      return quantize & kTransposeMask;
    }

    /**
     * @brief Encodes a phase [0..1) and a voice index into a single float, storing the voice in the integer portion.
     */
    force_inline poly_float encodePhaseAndVoice(poly_float phase, poly_float voice) {
      poly_float voice_float = toFloat((toInt(voice) & poly_int(kNotePressedMask)) + 1);
      return voice_float + phase * kPhaseEncodingMultiplier;
    }

    /**
     * @brief Decodes a phase and voice from an encoded float, returning (phase, voice).
     */
    force_inline std::pair<poly_float, poly_float> decodePhaseAndVoice(poly_float encoded) {
      poly_float modulo = mod(encoded);
      poly_float voice = encoded - modulo;
      poly_float phase = modulo * (1.0f / kPhaseEncodingMultiplier);
      return { phase, voice };
    }
  } // namespace utils
} // namespace vital

