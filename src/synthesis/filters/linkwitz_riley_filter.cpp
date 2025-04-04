#include "linkwitz_riley_filter.h"

namespace vital {

  /**
   * @brief Constructs a LinkwitzRileyFilter with an initial cutoff frequency and resets internal buffers.
   * @param cutoff The cutoff frequency in Hz.
   */
  LinkwitzRileyFilter::LinkwitzRileyFilter(mono_float cutoff)
      : Processor(kNumInputs, kNumOutputs) {
    cutoff_ = cutoff;

    // Initialize coefficients to zero
    low_in_0_ = low_in_1_ = low_in_2_ = 0.0f;
    low_out_1_ = low_out_2_ = 0.0f;
    high_in_0_ = high_in_1_ = high_in_2_ = 0.0f;
    high_out_1_ = high_out_2_ = 0.0f;

    // Reset filter buffers
    reset(constants::kFullMask);
  }

  /**
   * @brief Processes the input buffer for the specified number of samples,
   *        splitting it into low and high outputs.
   * @param num_samples Number of samples to process.
   */
  void LinkwitzRileyFilter::process(int num_samples) {
    processWithInput(input(kAudio)->source->buffer, num_samples);
  }

  /**
   * @brief Processes a given audio buffer, writing low-pass samples to kAudioLow
   *        and high-pass samples to kAudioHigh.
   * @param audio_in Pointer to the input audio buffer.
   * @param num_samples Number of samples to process.
   */
  void LinkwitzRileyFilter::processWithInput(const poly_float* audio_in, int num_samples) {
    // First pass: two cascaded low-pass filters

    // 1) First low-pass cascade
    poly_float* dest_low = output(kAudioLow)->buffer;
    for (int i = 0; i < num_samples; ++i) {
      // Current audio sample
      poly_float audio = audio_in[i];

      // Low-pass biquad structure
      // Stage A
      poly_float low_in01 = utils::mulAdd(audio * low_in_0_, past_in_1a_[kAudioLow], low_in_1_);
      poly_float low_in   = utils::mulAdd(low_in01, past_in_2a_[kAudioLow], low_in_2_);
      poly_float low_in_out1 = utils::mulAdd(low_in, past_out_1a_[kAudioLow], low_out_1_);
      poly_float low = utils::mulAdd(low_in_out1, past_out_2a_[kAudioLow], low_out_2_);

      // Update memory
      past_in_2a_[kAudioLow]  = past_in_1a_[kAudioLow];
      past_in_1a_[kAudioLow]  = audio;
      past_out_2a_[kAudioLow] = past_out_1a_[kAudioLow];
      past_out_1a_[kAudioLow] = low;

      // Write output to buffer (for the next cascade or final output)
      dest_low[i] = low;
    }

    // 2) Second low-pass cascade
    for (int i = 0; i < num_samples; ++i) {
      // Take the output from the first cascade
      poly_float audio = dest_low[i];

      poly_float low_in01 = utils::mulAdd(audio * low_in_0_, past_in_1b_[kAudioLow], low_in_1_);
      poly_float low_in   = utils::mulAdd(low_in01, past_in_2b_[kAudioLow], low_in_2_);
      poly_float low_in_out1 = utils::mulAdd(low_in, past_out_1b_[kAudioLow], low_out_1_);
      poly_float low = utils::mulAdd(low_in_out1, past_out_2b_[kAudioLow], low_out_2_);

      // Update memory
      past_in_2b_[kAudioLow]  = past_in_1b_[kAudioLow];
      past_in_1b_[kAudioLow]  = audio;
      past_out_2b_[kAudioLow] = past_out_1b_[kAudioLow];
      past_out_1b_[kAudioLow] = low;

      // Overwrite the dest_low buffer with the final low-pass result
      dest_low[i] = low;
    }

    // Second pass: two cascaded high-pass filters

    // 1) First high-pass cascade
    poly_float* dest_high = output(kAudioHigh)->buffer;
    for (int i = 0; i < num_samples; ++i) {
      poly_float audio = audio_in[i];

      // High-pass biquad structure
      // Stage A
      poly_float high_in01 = utils::mulAdd(audio * high_in_0_, past_in_1a_[kAudioHigh], high_in_1_);
      poly_float high_in   = utils::mulAdd(high_in01, past_in_2a_[kAudioHigh], high_in_2_);
      poly_float high_in_out1 = utils::mulAdd(high_in, past_out_1a_[kAudioHigh], high_out_1_);
      poly_float high = utils::mulAdd(high_in_out1, past_out_2a_[kAudioHigh], high_out_2_);

      // Update memory
      past_in_2a_[kAudioHigh]  = past_in_1a_[kAudioHigh];
      past_in_1a_[kAudioHigh]  = audio;
      past_out_2a_[kAudioHigh] = past_out_1a_[kAudioHigh];
      past_out_1a_[kAudioHigh] = high;

      dest_high[i] = high;
    }

    // 2) Second high-pass cascade
    for (int i = 0; i < num_samples; ++i) {
      poly_float audio = dest_high[i];

      poly_float high_in01 = utils::mulAdd(audio * high_in_0_, past_in_1b_[kAudioHigh], high_in_1_);
      poly_float high_in   = utils::mulAdd(high_in01, past_in_2b_[kAudioHigh], high_in_2_);
      poly_float high_in_out1 = utils::mulAdd(high_in, past_out_1b_[kAudioHigh], high_out_1_);
      poly_float high = utils::mulAdd(high_in_out1, past_out_2b_[kAudioHigh], high_out_2_);

      // Update memory
      past_in_2b_[kAudioHigh]  = past_in_1b_[kAudioHigh];
      past_in_1b_[kAudioHigh]  = audio;
      past_out_2b_[kAudioHigh] = past_out_1b_[kAudioHigh];
      past_out_1b_[kAudioHigh] = high;

      dest_high[i] = high;
    }
  }

  /**
   * @brief Calculates biquad coefficients for the Linkwitz-Riley filter
   *        based on the current cutoff frequency and sample rate.
   */
  void LinkwitzRileyFilter::computeCoefficients() {
    // Warp frequency using bilinear transform
    mono_float warp = 1.0f / tanf(kPi * cutoff_ / getSampleRate());
    mono_float warp2 = warp * warp;
    mono_float mult = 1.0f / (1.0f + kSqrt2 * warp + warp2);

    // Low-pass filter coefficients
    low_in_0_ = mult;
    low_in_1_ = 2.0f * mult;
    low_in_2_ = mult;
    low_out_1_ = -2.0f * (1.0f - warp2) * mult;
    low_out_2_ = -(1.0f - kSqrt2 * warp + warp2) * mult;

    // High-pass filter coefficients
    high_in_0_ = warp2 * mult;
    high_in_1_ = -2.0f * high_in_0_;
    high_in_2_ = high_in_0_;
    high_out_1_ = low_out_1_;
    high_out_2_ = low_out_2_;
  }

  /**
   * @brief Sets the new sample rate and recalculates the filter coefficients.
   * @param sample_rate The sample rate in Hz.
   */
  void LinkwitzRileyFilter::setSampleRate(int sample_rate) {
    Processor::setSampleRate(sample_rate);
    computeCoefficients();
  }

  /**
   * @brief Sets the new oversampling amount and recalculates the filter coefficients.
   * @param oversample_amount The oversampling factor.
   */
  void LinkwitzRileyFilter::setOversampleAmount(int oversample_amount) {
    Processor::setOversampleAmount(oversample_amount);
    computeCoefficients();
  }

  /**
   * @brief Resets filter memories (delay lines) for the specified voices.
   * @param reset_mask A bitmask indicating which voices to reset.
   */
  void LinkwitzRileyFilter::reset(poly_mask reset_mask) {
    for (int i = 0; i < kNumOutputs; ++i) {
      past_in_1a_[i]  = utils::maskLoad(past_in_1a_[i], 0.0f, reset_mask);
      past_in_2a_[i]  = utils::maskLoad(past_in_2a_[i], 0.0f, reset_mask);
      past_out_1a_[i] = utils::maskLoad(past_out_1a_[i], 0.0f, reset_mask);
      past_out_2a_[i] = utils::maskLoad(past_out_2a_[i], 0.0f, reset_mask);

      past_in_1b_[i]  = utils::maskLoad(past_in_1b_[i], 0.0f, reset_mask);
      past_in_2b_[i]  = utils::maskLoad(past_in_2b_[i], 0.0f, reset_mask);
      past_out_1b_[i] = utils::maskLoad(past_out_1b_[i], 0.0f, reset_mask);
      past_out_2b_[i] = utils::maskLoad(past_out_2b_[i], 0.0f, reset_mask);
    }
  }
} // namespace vital
