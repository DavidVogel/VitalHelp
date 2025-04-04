#pragma once

/**
 * @file utils.h
 * @brief Provides various utility functions, classes, and constants for
 *        audio, math, and general-purpose operations within Vital.
 */

#include "common.h"

#include <cmath>
#include <complex>
#include <cstdlib>
#include <random>

namespace vital {

  /**
   * @namespace utils
   * @brief Contains a collection of utility functions and classes used throughout Vital.
   */
  namespace utils {

    /// Multiplicative factor for converting dB <-> magnitude using dB = 20*log10(magnitude).
    constexpr mono_float kDbGainConversionMult = 20.0f;
    /// Maximum length for orders that can be encoded as a float via encodeOrderToFloat().
    constexpr int kMaxOrderLength = 10;
    /// Natural log of 2.
    constexpr mono_float kLogOf2 = 0.69314718056f;
    /// Reciprocal of the natural log of 2 (1 / ln(2)).
    constexpr mono_float kInvLogOf2 = 1.44269504089f;

    /**
     * @brief Computes the factorial of a given integer at compile time.
     * @param value The integer whose factorial is needed.
     * @return The factorial of @p value.
     */
    constexpr int factorial(int value) {
      int result = 1;
      for (int i = 2; i <= value; ++i)
        result *= i;
      return result;
    }

    /**
     * @union int_float
     * @brief A small union to reinterpret an int as a float or vice versa.
     */
    typedef union {
      int i;        ///< Integer representation.
      mono_float f; ///< Float representation.
    } int_float;

    /**
     * @class RandomGenerator
     * @brief A basic random number generator for producing uniform distributions of floats.
     *
     * It uses an internal Mersenne Twister (`std::mt19937`) and a uniform distribution
     * between given min and max values.
     */
    class RandomGenerator {
      public:
        /// Static seed counter used to automatically assign seeds if none specified.
        static int next_seed_;

        /**
         * @brief Constructs a RandomGenerator with specified min and max values.
         * @param min The minimum floating-point value (inclusive).
         * @param max The maximum floating-point value (inclusive).
         */
        RandomGenerator(mono_float min, mono_float max)
          : engine_(next_seed_++), distribution_(min, max) { }

        /**
         * @brief Copy constructor, but it re-seeds the engine for uniqueness.
         * @param other Another RandomGenerator to copy min/max from.
         */
        RandomGenerator(const RandomGenerator& other)
          : engine_(next_seed_++),
            distribution_(other.distribution_.min(), other.distribution_.max()) { }

        /**
         * @brief Returns the next random float in [min, max].
         */
        force_inline mono_float next() {
          return distribution_(engine_);
        }

        /**
         * @brief Produces a poly_float with random values in each lane.
         * @return A poly_float of random values (one random value per lane).
         */
        force_inline poly_float polyNext() {
          poly_float result;
          for (int i = 0; i < poly_float::kSize; ++i)
            result.set(i, next());
          return result;
        }

        /**
         * @brief Produces a poly_float with random values assigned in pairs
         *        (every 2 lanes share the same random value).
         * @return A poly_float of random values in pairs.
         */
        force_inline poly_float polyVoiceNext() {
          poly_float result;
          for (int i = 0; i < poly_float::kSize; i += 2) {
            mono_float value = next();
            result.set(i, value);
            result.set(i + 1, value);
          }
          return result;
        }

        /**
         * @brief Produces a poly_float of random values, only generated for lanes set in @p mask.
         * @param mask A poly_mask indicating which lanes need new random values.
         * @return A poly_float with random values in masked lanes, zero in others.
         */
        force_inline poly_float polyNext(poly_mask mask) {
          poly_float result = 0.0f;
          for (int i = 0; i < poly_float::kSize; ++i) {
            if (mask[i])
              result.set(i, next());
          }
          return result;
        }

        /**
         * @brief Reseeds the internal random engine with @p new_seed.
         * @param new_seed The new seed to use.
         */
        force_inline void seed(int new_seed) {
          engine_.seed(new_seed);
        }

      private:
        std::mt19937 engine_;                                      ///< Mersenne Twister engine.
        std::uniform_real_distribution<mono_float> distribution_;  ///< Uniform distribution.

        JUCE_LEAK_DETECTOR(RandomGenerator)
    };

    /**
     * @brief Reinterprets an int as a float (bitwise).
     * @param i The int to reinterpret.
     * @return The float representation of the same bits.
     */
    force_inline mono_float intToFloatBits(int i) {
      int_float convert;
      convert.i = i;
      return convert.f;
    }

    /**
     * @brief Reinterprets a float as an int (bitwise).
     * @param f The float to reinterpret.
     * @return The int representation of the same bits.
     */
    force_inline int floatToIntBits(mono_float f) {
      int_float convert;
      convert.f = f;
      return convert.i;
    }

    /**
     * @brief Returns the minimum of two floats.
     */
    force_inline mono_float min(mono_float one, mono_float two) {
      return fmin(one, two);
    }

    /**
     * @brief Returns the maximum of two floats.
     */
    force_inline mono_float max(mono_float one, mono_float two) {
      return fmax(one, two);
    }

    /**
     * @brief Clamps a value between [min_val, max_val].
     * @param value    The value to clamp.
     * @param min_val  The minimum bound.
     * @param max_val  The maximum bound.
     * @return The clamped value.
     */
    force_inline mono_float clamp(mono_float value, mono_float min_val, mono_float max_val) {
      return fmin(max_val, fmax(value, min_val));
    }

    /**
     * @brief A pass-through function that simply returns the input. Often used in templated code.
     * @tparam T Generic type.
     * @param input The input value.
     * @return The same input value.
     */
    template<class T>
    force_inline T pass(T input) {
      return input;
    }

    /**
     * @brief Returns the maximum of two integers.
     */
    force_inline int imax(int one, int two) {
      return (one > two) ? one : two;
    }

    /**
     * @brief Returns the minimum of two integers.
     */
    force_inline int imin(int one, int two) {
      return (one > two) ? two : one;
    }

    /**
     * @brief Linearly interpolates between two double values.
     * @param from The starting value.
     * @param to   The ending value.
     * @param t    A fraction in [0, 1], where 0 returns @p from and 1 returns @p to.
     */
    force_inline double interpolate(double from, double to, double t) {
      return t * (to - from) + from;
    }

    /**
     * @brief Linearly interpolates between two floats.
     */
    force_inline mono_float interpolate(mono_float from, mono_float to, mono_float t) {
      return from + t * (to - from);
    }

    /**
     * @brief Returns the fractional part of @p value, storing the integer part in @p divisor (double).
     */
    force_inline mono_float mod(double value, double* divisor) {
      return modf(value, divisor);
    }

    /**
     * @brief Returns the fractional part of @p value, storing the integer part in @p divisor (float).
     */
    force_inline mono_float mod(float value, float* divisor) {
      return modff(value, divisor);
    }

    /**
     * @brief Clamps an integer between [min_val, max_val].
     */
    force_inline int iclamp(int value, int min_val, int max_val) {
      return value > max_val ? max_val : (value < min_val ? min_val : value);
    }

    /**
     * @brief Computes the floor of the base-2 logarithm of an integer (effectively the index of the highest set bit).
     * @param value The integer to compute log2 for (must be > 0).
     * @return The integer part of log2(value).
     */
    force_inline int ilog2(int value) {
    #if defined(__GNUC__) || defined(__clang__)
      constexpr int kMaxBitIndex = sizeof(int) * 8 - 1;
      return kMaxBitIndex - __builtin_clz(std::max(value, 1));
    #elif defined(_MSC_VER)
      unsigned long result = 0;
      _BitScanReverse(&result, value);
      return result;
    #else
      int num = 0;
      while (value >>= 1)
        num++;
      return num;
    #endif
    }

    /**
     * @brief Determines if a float is close to zero (within ±kEpsilon).
     * @param value The float to test.
     * @return True if |value| < kEpsilon, false otherwise.
     */
    force_inline bool closeToZero(mono_float value) {
      return value <= kEpsilon && value >= -kEpsilon;
    }

    /**
     * @brief Converts a magnitude to decibels using 20*log10(magnitude).
     * @param magnitude The non-negative amplitude or magnitude.
     * @return The dB value.
     */
    force_inline mono_float magnitudeToDb(mono_float magnitude) {
      return kDbGainConversionMult * log10f(magnitude);
    }

    /**
     * @brief Converts decibels to linear magnitude using 10^(dB / 20).
     * @param decibels The dB value.
     * @return The corresponding magnitude.
     */
    force_inline mono_float dbToMagnitude(mono_float decibels) {
      return powf(10.0f, decibels / kDbGainConversionMult);
    }

    /**
     * @brief Converts cents to a ratio. A value of 1200 cents is 2.0.
     * @param cents The number of cents (1 semitone = 100 cents).
     * @return A ratio of the frequency.
     */
    force_inline mono_float centsToRatio(mono_float cents) {
      return powf(2.0f, cents / kCentsPerOctave);
    }

    /**
     * @brief Converts a note offset in semitones (or partial) to a frequency ratio.
     * @param cents The number of semitones (100 cents each).
     * @return The resulting ratio.
     */
    force_inline mono_float noteOffsetToRatio(mono_float cents) {
      return powf(2.0f, cents / kNotesPerOctave);
    }

    /**
     * @brief Converts a frequency ratio to a MIDI transpose value (in semitones).
     * @param ratio The frequency ratio (e.g., 2.0 for one octave above).
     * @return Number of semitones (MIDI note difference).
     */
    force_inline mono_float ratioToMidiTranspose(mono_float ratio) {
      return logf(ratio) * (kInvLogOf2 * kNotesPerOctave);
    }

    /**
     * @brief Converts a MIDI-based cents value to an absolute frequency, relative to MIDI note 0.
     * @param cents The note offset in cents relative to MIDI note 0.
     * @return Frequency in Hz.
     */
    force_inline mono_float midiCentsToFrequency(mono_float cents) {
      return kMidi0Frequency * centsToRatio(cents);
    }

    /**
     * @brief Converts a MIDI note number to frequency in Hz.
     * @param note MIDI note in semitones (C-1 is 0, A4 is 69, etc.).
     * @return The frequency in Hz.
     */
    force_inline mono_float midiNoteToFrequency(mono_float note) {
      return midiCentsToFrequency(note * kCentsPerNote);
    }

    /**
     * @brief Converts a frequency in Hz to a MIDI note number.
     * @param frequency The frequency in Hz.
     * @return The approximate MIDI note number.
     */
    force_inline mono_float frequencyToMidiNote(mono_float frequency) {
      return kNotesPerOctave * logf(frequency / kMidi0Frequency) * kInvLogOf2;
    }

    /**
     * @brief Converts a frequency in Hz to MIDI cents relative to MIDI note 0.
     * @param frequency The frequency in Hz.
     * @return The cents value (1 semitone = 100).
     */
    force_inline mono_float frequencyToMidiCents(mono_float frequency) {
      return kCentsPerNote * frequencyToMidiNote(frequency);
    }

    /**
     * @brief Finds the next power of two greater than or equal to a float value.
     * @param value A float specifying the lower bound.
     * @return An integer that is a power of two >= @p value.
     */
    force_inline int nextPowerOfTwo(mono_float value) {
      return roundf(powf(2.0f, ceilf(logf(value) * kInvLogOf2)));
    }

    /**
     * @brief Checks if all samples in a buffer are close to zero.
     * @param buffer A pointer to an array of mono_float samples.
     * @param length The number of samples in the buffer.
     * @return True if all samples are near zero, false otherwise.
     */
    force_inline bool isSilent(const mono_float* buffer, int length) {
      for (int i = 0; i < length; ++i) {
        if (!closeToZero(buffer[i]))
          return false;
      }
      return true;
    }

    /**
     * @brief Computes the Root Mean Square (RMS) of a buffer of floats.
     * @param buffer The pointer to the samples.
     * @param num    The number of samples.
     * @return The RMS of the buffer.
     */
    force_inline mono_float rms(const mono_float* buffer, int num) {
      mono_float square_total = 0.0f;
      for (int i = 0; i < num; ++i)
        square_total += buffer[i] * buffer[i];

      return sqrtf(square_total / num);
    }

    /**
     * @brief A curve function used for specific shaping or scaling of a parameter.
     * @param t Input value in [0, 1].
     * @return A scaled value, used in certain curve transformations.
     */
    force_inline mono_float inversePowerScale(mono_float t) {
      return 2.0f * logf((-t + 1.0f) / t);
    }

    /**
     * @brief Another curve function, typically used for certain shape transformations.
     * @param t Input value in [0, 1].
     * @return A scaled result.
     */
    force_inline mono_float inverseFltScale(mono_float t) {
      return (t - 1.0f) / t;
    }

    /**
     * @brief Encodes a permutation order array into a single float (bit-packed).
     * @param order An integer array representing the permutation order.
     * @param size  The length of the array (up to kMaxOrderLength).
     * @return A float representation of the permutation.
     */
    mono_float encodeOrderToFloat(int* order, int size);

    /**
     * @brief Decodes a float-encoded permutation (from encodeOrderToFloat) back into an integer array.
     * @param order      The integer array to fill with the permutation.
     * @param float_code The float code containing the encoded order.
     * @param size       The length of the order array.
     */
    void decodeFloatToOrder(int* order, mono_float float_code, int size);

    /**
     * @brief Converts floating-point audio data to 16-bit PCM data.
     * @param pcm_data   Destination PCM array.
     * @param float_data Source float array.
     * @param size       Number of samples to convert.
     */
    void floatToPcmData(int16_t* pcm_data, const float* float_data, int size);

    /**
     * @brief Converts an array of complex floats (magnitude/phase) to PCM data.
     * @param pcm_data     Destination PCM array (size must be at least @p size).
     * @param complex_data Source complex float array.
     * @param size         Number of samples in the complex array (note that real PCM size is double).
     */
    void complexToPcmData(int16_t* pcm_data, const std::complex<float>* complex_data, int size);

    /**
     * @brief Converts 16-bit PCM data to floating-point audio data.
     * @param float_data Destination float array.
     * @param pcm_data   Source PCM array.
     * @param size       Number of samples to convert.
     */
    void pcmToFloatData(float* float_data, const int16_t* pcm_data, int size);

    /**
     * @brief Converts 16-bit PCM data representing complex info (amp/phase) back to std::complex floats.
     * @param complex_data Destination complex float array.
     * @param pcm_data     Source PCM array.
     * @param size         Number of samples in @p pcm_data (note that real complex size is half).
     */
    void pcmToComplexData(std::complex<float>* complex_data, const int16_t* pcm_data, int size);

  } // namespace utils
} // namespace vital
