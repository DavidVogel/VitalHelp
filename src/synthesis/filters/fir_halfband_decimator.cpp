#include "fir_halfband_decimator.h"
#include "poly_utils.h"

namespace vital {

  /**
   * @brief Constructs the FirHalfbandDecimator, setting up FIR coefficients and resetting memory.
   *
   * The static array `coefficients` stores the full 32 taps. Half (kNumTaps/2) are packed into
   * each element of `taps_` as a poly_float, allowing pairwise processing in the decimation loop.
   */
  FirHalfbandDecimator::FirHalfbandDecimator() : Processor(kNumInputs, 1) {
    static const mono_float coefficients[kNumTaps] = {
      0.000088228877315364f,  0.000487010018128278f,  0.000852264975437944f,  -0.001283563593466774f,
      -0.010130591831925894f, -0.025688727779244691f, -0.036346596505004387f, -0.024088355516718698f,
      0.012246773417129486f,  0.040021434054637831f,  0.017771298164062477f,  -0.046866403416502632f,
      -0.075597513455990611f,  0.013331126342402619f,  0.202889888191404910f,  0.362615173769444080f,
      0.362615173769444080f,  0.202889888191404910f,  0.013331126342402619f,  -0.075597513455990611f,
      -0.046866403416502632f,  0.017771298164062477f,  0.040021434054637831f,  0.012246773417129486f,
      -0.024088355516718698f, -0.036346596505004387f, -0.025688727779244691f, -0.010130591831925894f,
      -0.001283563593466774f,  0.000852264975437944f,  0.000487010018128278f,  0.000088228877315364f,
    };

    // Pack every pair of coefficients into taps_
    for (int i = 0; i < kNumTaps / 2; ++i)
      taps_[i] = poly_float(coefficients[2 * i], coefficients[2 * i + 1]);

    reset(constants::kFullMask);
  }

  /**
   * @brief Saves samples from the tail of the current processing block to memory, so the filter can continue seamlessly.
   * @param num_samples The number of output samples processed in this block (the input is 2 * num_samples).
   *
   * This function copies the last few input samples into the `memory_` array to be used as the first
   * samples in the next block's processing. This prevents discontinuities at block boundaries.
   */
  void FirHalfbandDecimator::saveMemory(int num_samples) {
    int input_buffer_size = 2 * num_samples;
    poly_float* audio = input(kAudio)->source->buffer;

    // Compute where in the input buffer we should start saving
    int start_audio_index = input_buffer_size - kNumTaps + 2;
    for (int i = 0; i < kNumTaps / 2 - 1; ++i) {
      int audio_index = start_audio_index + 2 * i;
      // Consolidate two channels/samples at a time for poly_float
      memory_[i] = utils::consolidateAudio(audio[audio_index], audio[audio_index + 1]);
    }
  }

  /**
   * @brief Processes the audio by decimating it (reducing sample rate by half) using a half-band FIR filter.
   * @param num_samples The number of output samples to produce. The input must have 2 * num_samples samples.
   *
   * For each output sample, the filter sums products of input samples and filter coefficients,
   * then uses the `utils::sumSplitAudio()` function to sum the two lanes of the poly_float result.
   */
  void FirHalfbandDecimator::process(int num_samples) {
    const poly_float* audio = input(kAudio)->source->buffer;
    int output_buffer_size = num_samples;

    // Basic checks to ensure buffer sizes are valid
    VITAL_ASSERT(output_buffer_size > kNumTaps / 2);
    VITAL_ASSERT(input(kAudio)->source->buffer_size >= 2 * output_buffer_size);

    poly_float* audio_out = output()->buffer;

    // First, use memory_ from the previous block to fill the early samples
    for (int memory_start = 0; memory_start < kNumTaps / 2 - 1; ++memory_start) {
      poly_float sum = 0.0f;

      // Use stored memory first
      int tap_index = 0;
      int num_memory = kNumTaps / 2 - memory_start - 1;
      for (; tap_index < num_memory; ++tap_index) {
        sum = utils::mulAdd(sum, memory_[tap_index + memory_start], taps_[tap_index]);
      }

      // Then move on to the current block's audio
      int audio_index = 0;
      for (; tap_index < kNumTaps / 2; ++tap_index) {
        // Consolidate two adjacent samples into a poly_float
        poly_float consolidated = utils::consolidateAudio(audio[audio_index], audio[audio_index + 1]);
        sum = utils::mulAdd(sum, consolidated, taps_[tap_index]);
        audio_index += 2;
      }

      // Write out a decimated sample (sum both lanes)
      audio_out[memory_start] = utils::sumSplitAudio(sum);
    }

    // Process the rest of the block
    int out_index = kNumTaps / 2 - 1;
    int audio_start = 0;
    for (; out_index < output_buffer_size; ++out_index) {
      poly_float sum = 0.0f;
      int audio_index = audio_start;

      // Sum over all taps for each output sample
      for (int tap_index = 0; tap_index < kNumTaps / 2; ++tap_index) {
        poly_float consolidated = utils::consolidateAudio(audio[audio_index], audio[audio_index + 1]);
        sum = utils::mulAdd(sum, consolidated, taps_[tap_index]);
        audio_index += 2;
      }
      audio_start += 2;

      audio_out[out_index] = utils::sumSplitAudio(sum);
    }

    // Save the end of this block to memory for seamless continuity next time
    saveMemory(num_samples);
  }

  /**
   * @brief Resets the internal memory for all voices indicated by the mask.
   * @param reset_mask A bitmask indicating which voices to reset (unused in this filter).
   *
   * Sets the `memory_` samples to zero, ensuring no residual data from a previous note or block remains.
   */
  void FirHalfbandDecimator::reset(poly_mask reset_mask) {
    for (int i = 0; i < kNumTaps / 2 - 1; ++i)
      memory_[i] = 0.0f;
  }

} // namespace vital
