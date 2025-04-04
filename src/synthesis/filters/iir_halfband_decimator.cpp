#include "iir_halfband_decimator.h"

namespace vital {

  /**
   * @brief Coefficients for the 9-tap IIR half-band filter, stored in poly_float pairs.
   *
   * Each pair contains coefficients for two parallel paths (or for two lanes in poly_float).
   */
  poly_float IirHalfbandDecimator::kTaps9[kNumTaps9] = {
    { 0.167135116548925f, 0.0413554705262319f },
    { 0.742130012538075f, 0.3878932830211427f },
  };

  /**
   * @brief Coefficients for the 25-tap IIR half-band filter, stored in poly_float pairs.
   *
   * Each pair contains coefficients for two parallel paths (or for two lanes in poly_float).
   */
  poly_float IirHalfbandDecimator::kTaps25[kNumTaps25] = {
    { 0.093022421467960f, 0.024388383731296f },
    { 0.312318050871736f, 0.194029987625265f },
    { 0.548379093159427f, 0.433855675727187f },
    { 0.737198546150414f, 0.650124972769370f },
    { 0.872234992057129f, 0.810418671775866f },
    { 0.975497791832324f, 0.925979700943193f }
  };

  /**
   * @brief Constructor for IirHalfbandDecimator, defaults to the 9-tap (lighter) filter mode.
   */
  IirHalfbandDecimator::IirHalfbandDecimator()
      : Processor(kNumInputs, 1), sharp_cutoff_(false) {
    reset(constants::kFullMask);
  }

  /**
   * @brief Processes the audio block, decimating by a factor of 2.
   * @param num_samples Number of output samples. The input buffer must have 2 * num_samples samples.
   *
   * Applies an IIR half-band filter with either 9 or 25 taps, depending on @c sharp_cutoff_.
   * The result is written to the output buffer, halving the sample rate of the incoming signal.
   */
  void IirHalfbandDecimator::process(int num_samples) {
    int num_taps = kNumTaps9;
    const poly_float* taps = kTaps9;
    if (sharp_cutoff_) {
      num_taps = kNumTaps25;
      taps = kTaps25;
    }

    const poly_float* audio = input(kAudio)->source->buffer;
    int output_buffer_size = num_samples;
    VITAL_ASSERT(input(kAudio)->source->buffer_size >= 2 * output_buffer_size);

    poly_float* audio_out = output()->buffer;

    // Main loop: for each output sample, combine two input samples and update the delay lines
    for (int i = 0; i < output_buffer_size; ++i) {
      int audio_in_index = 2 * i;

      // Consolidate two adjacent input samples into one poly_float
      poly_float result = utils::consolidateAudio(audio[audio_in_index],
                                                  audio[audio_in_index + 1]);

      // Run through each tap in the IIR filter
      for (int tap_index = 0; tap_index < num_taps; ++tap_index) {
        // Compute difference between current signal and stored output
        poly_float delta = result - out_memory_[tap_index];
        // Multiply-accumulate with the tap coefficient
        poly_float new_result = utils::mulAdd(in_memory_[tap_index], taps[tap_index], delta);

        // Update delay lines
        in_memory_[tap_index] = result;
        out_memory_[tap_index] = new_result;

        // The filter output at this stage
        result = new_result;
      }

      // Sum the poly_float lanes and scale by 0.5f
      audio_out[i] = utils::sumSplitAudio(result) * 0.5f;
    }
  }

  /**
   * @brief Resets the decimator's internal memory for the specified voices.
   * @param reset_mask A bitmask specifying which voices to reset (unused here).
   *
   * Clears all delay line states to 0, ensuring no leftover data affects the next block.
   */
  void IirHalfbandDecimator::reset(poly_mask reset_mask) {
    for (int i = 0; i < kNumTaps25; ++i) {
      in_memory_[i] = 0.0f;
      out_memory_[i] = 0.0f;
    }
  }
} // namespace vital
