#include "dc_filter.h"

namespace vital {

  /**
   * @brief Constructs a DcFilter and initializes internal states.
   */
  DcFilter::DcFilter() : Processor(DcFilter::kNumInputs, 1) {
    coefficient_ = 0.0f;
    reset(constants::kFullMask);
  }

  /**
   * @brief Updates the sample rate and recalculates the filter coefficient.
   *
   * @param sample_rate The new sample rate in Hz.
   */
  void DcFilter::setSampleRate(int sample_rate) {
    Processor::setSampleRate(sample_rate);
    coefficient_ = 1.0f - kCoefficientToSrConstant / getSampleRate();
  }

  /**
   * @brief Processes a block of samples from the primary kAudio input.
   *
   * @param num_samples Number of samples to process.
   */
  void DcFilter::process(int num_samples) {
    VITAL_ASSERT(inputMatchesBufferSize(kAudio));
    processWithInput(input(kAudio)->source->buffer, num_samples);
  }

  /**
   * @brief Processes a block of samples with the given input buffer and writes to the output buffer.
   *
   * @param audio_in    Pointer to the input buffer.
   * @param num_samples Number of samples to process.
   */
  void DcFilter::processWithInput(const poly_float* audio_in, int num_samples) {
    // Check if we need to reset any voices
    poly_mask reset_mask = getResetMask(kReset);
    if (reset_mask.anyMask())
      reset(reset_mask);

    poly_float* audio_out = output()->buffer;
    for (int i = 0; i < num_samples; ++i)
      tick(audio_in[i], audio_out[i]);
  }

  /**
   * @brief Processes a single sample, removing DC offset via a one-pole high-pass filter.
   *
   * @param audio_in  The input sample.
   * @param audio_out Reference to the output sample.
   */
  force_inline void DcFilter::tick(const poly_float& audio_in, poly_float& audio_out) {
    // y[n] = y[n-1] + (x[n] - x[n-1]) * coefficient
    audio_out = utils::mulAdd(audio_in - past_in_, past_out_, coefficient_);
    past_out_ = audio_out;
    past_in_ = audio_in;
  }

  /**
   * @brief Resets the filter state (past input and output) for the specified voices.
   *
   * @param reset_mask poly_mask indicating which voices to reset.
   */
  void DcFilter::reset(poly_mask reset_mask) {
    past_in_ = utils::maskLoad(past_in_, 0.0f, reset_mask);
    past_out_ = utils::maskLoad(past_in_, 0.0f, reset_mask);
  }

} // namespace vital
