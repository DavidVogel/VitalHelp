#include "phaser_filter.h"

#include "futils.h"

namespace vital {

  /**
   * @brief Constructs a PhaserFilter object with the specified mode (clean or distorted).
   * @param clean If true, uses a mild saturation in the resonance path; otherwise uses a more intense saturation.
   */
  PhaserFilter::PhaserFilter(bool clean) : Processor(PhaserFilter::kNumInputs, 1) {
    clean_ = clean;
    hardReset();
    // Default to no inversion of the phaser output
    invert_mult_ = 1.0f;
  }

  /**
   * @brief Resets internal phaser states for the voices specified by reset_mask.
   * @param reset_mask A bitmask indicating which voices to reset.
   */
  void PhaserFilter::reset(poly_mask reset_mask) {
    allpass_output_ = utils::maskLoad(allpass_output_, 0.0f, reset_mask);
    for (int i = 0; i < kMaxStages; ++i)
      stages_[i].reset(reset_mask);

    remove_lows_stage_.reset(reset_mask);
    remove_highs_stage_.reset(reset_mask);
  }

  /**
   * @brief Performs a full reset of all phaser states, clearing resonance, drive, and peak amounts.
   */
  void PhaserFilter::hardReset() {
    reset(constants::kFullMask);
    resonance_ = 0.0f;
    drive_ = 0.0f;
    peak1_amount_ = 0.0f;
    peak3_amount_ = 0.0f;
    peak5_amount_ = 0.0f;
    allpass_output_ = 0.0f;
  }

  /**
   * @brief Processes a block of audio through the phaser effect.
   * @param num_samples Number of samples to process.
   */
  void PhaserFilter::process(int num_samples) {
    VITAL_ASSERT(inputMatchesBufferSize(kAudio));
    processWithInput(input(kAudio)->source->buffer, num_samples);
  }

  /**
   * @brief Processes the provided input buffer through either the "clean" or more saturated path.
   * @param audio_in The input audio buffer.
   * @param num_samples Number of samples to process.
   */
  void PhaserFilter::processWithInput(const poly_float* audio_in, int num_samples) {
    // If clean_ is true, saturate resonance path with tanh; input passes through without saturation (utils::pass).
    // Otherwise, let resonance pass unaltered (utils::pass), saturating the input with hardTanh.
    if (clean_)
      process<futils::tanh, utils::pass>(audio_in, num_samples);
    else
      process<utils::pass, futils::hardTanh>(audio_in, num_samples);
  }

  /**
   * @brief Updates internal phaser parameters from a FilterState object.
   * @param filter_state The new settings to apply (e.g., resonance, drive, pass_blend).
   */
  void PhaserFilter::setupFilter(const FilterState& filter_state) {
    // Clamp resonance_percent between 0.0 and 1.0
    poly_float resonance_percent = utils::clamp(filter_state.resonance_percent, 0.0f, 1.0f);
    resonance_ = utils::interpolate(kMinResonance, kMaxResonance, resonance_percent);

    // Drive is scaled by the resonance factor for some additional feedback
    drive_ = (resonance_ * 0.5f + 1.0f) * filter_state.drive;

    // pass_blend dictates how to distribute peaks among 1, 3, and 5
    poly_float blend = filter_state.pass_blend;
    peak1_amount_ = utils::clamp(-blend + 1.0f, 0.0f, 1.0f);
    peak5_amount_ = utils::clamp(blend - 1.0f, 0.0f, 1.0f);
    // peak3_amount_ is whatever remains to ensure the sum is 1
    peak3_amount_ = -peak1_amount_ - peak5_amount_ + 1.0f;

    // If style is non-zero, invert the phaser output; otherwise, do not invert.
    if (filter_state.style)
      invert_mult_ = -1.0f;
    else
      invert_mult_ = 1.0f;
  }

} // namespace vital
