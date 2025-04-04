/**
 * @file utils.cpp
 * @brief Implements utility functions and classes declared in utils.h,
 *        including PCM conversion, order encoding/decoding, random generation, etc.
 */

#include "utils.h"

namespace vital {

  /// Scaling for converting float data to 16-bit PCM.
  constexpr float kPcmScale = 32767.0f;
  /// Additional amplitude scale used for complex data to PCM.
  constexpr float kComplexAmplitudePcmScale = 50.0f;
  /// Additional phase scale used for complex data to PCM.
  constexpr float kComplexPhasePcmScale = 10000.0f;

  namespace utils {

    /// Initializes the static seed counter for RandomGenerator.
    int RandomGenerator::next_seed_ = 0;

    /**
     * @brief Encodes a permutation (stored in @p order) into a single float.
     * @param order Integer array representing the permutation.
     * @param size  Length of the array (up to kMaxOrderLength).
     * @return Encoded float.
     *
     * Each element in @p order is used to compute a factorial-based integer code.
     * It is then returned as a float.
     */
    mono_float encodeOrderToFloat(int* order, int size) {
      // Max array size you can encode in 32 bits.
      VITAL_ASSERT(size <= kMaxOrderLength);

      unsigned int code = 0;
      for (int i = 1; i < size; ++i) {
        int index = 0;
        for (int j = 0; j < i; ++j)
          index += order[i] < order[j];

        code *= i + 1;
        code += index;
      }

      return code;
    }

    /**
     * @brief Decodes a float-encoded permutation back into @p order.
     * @param order      The integer array to fill.
     * @param float_code The encoded float from encodeOrderToFloat.
     * @param size       Number of elements in @p order.
     */
    void decodeFloatToOrder(int* order, mono_float float_code, int size) {
      // Max array size you can encode in 32 bits.
      VITAL_ASSERT(size <= kMaxOrderLength);

      int code = static_cast<int>(float_code);
      for (int i = 0; i < size; ++i)
        order[i] = i;

      for (int i = 0; i < size; ++i) {
        int remaining = size - i;
        int index = remaining - 1;
        int inversions = code % remaining;
        code /= remaining;

        int placement = order[index - inversions];
        for (int j = index - inversions; j < index; ++j)
          order[j] = order[j + 1];

        order[index] = placement;
      }
    }

    void floatToPcmData(int16_t* pcm_data, const float* float_data, int size) {
      for (int i = 0; i < size; ++i)
        pcm_data[i] = utils::clamp(float_data[i] * kPcmScale, -kPcmScale, kPcmScale);
    }

    void complexToPcmData(int16_t* pcm_data, const std::complex<float>* complex_data, int size) {
      // Interprets the complex data as (amplitude, phase) pairs.
      // The size argument is the total number of float samples in complex_data,
      // but each complex entry uses 2 in PCM (amp, phase).
      for (int i = 0; i < size / 2; ++i) {
        float amp = std::abs(complex_data[i]);
        float phase = std::arg(complex_data[i]);
        pcm_data[i * 2] = utils::clamp(amp * kComplexAmplitudePcmScale, -kPcmScale, kPcmScale);
        pcm_data[i * 2 + 1] = utils::clamp(phase * kComplexPhasePcmScale, -kPcmScale, kPcmScale);
      }
    }

    void pcmToFloatData(float* float_data, const int16_t* pcm_data, int size) {
      for (int i = 0; i < size; ++i)
        float_data[i] = pcm_data[i] * (1.0f / kPcmScale);
    }

    void pcmToComplexData(std::complex<float>* complex_data, const int16_t* pcm_data, int size) {
      // Inverse of complexToPcmData: decode amplitude and phase from PCM to std::polar form.
      for (int i = 0; i < size / 2; ++i) {
        float amp = pcm_data[i * 2] * (1.0f / kComplexAmplitudePcmScale);
        float phase = pcm_data[i * 2 + 1] * (1.0f / kComplexPhasePcmScale);
        complex_data[i] = std::polar(amp, phase);
      }
    }

  } // namespace utils
} // namespace vital
