#include "diode_filter.h"
#include "futils.h"

namespace vital {

  /**
   * @brief Constructs a DiodeFilter with the necessary number of inputs, then performs a hard reset.
   */
  DiodeFilter::DiodeFilter() : Processor(DiodeFilter::kNumInputs, 1) {
    hardReset();
  }

  /**
   * @brief Resets internal states of the filter based on the provided mask.
   * @param reset_mask A bitmask indicating which voices should be reset.
   */
  void DiodeFilter::reset(poly_mask reset_mask) {
    high_pass_1_.reset(reset_mask);
    high_pass_2_.reset(reset_mask);
    stage1_.reset(reset_mask);
    stage2_.reset(reset_mask);
    stage3_.reset(reset_mask);
    stage4_.reset(reset_mask);
  }

  /**
   * @brief Performs a hard reset on all internal parameters and states (for all voices).
   *
   * Resets the filter’s internal states and zeroes out any custom parameters like resonance, drive,
   * and post-multiply factor.
   */
  void DiodeFilter::hardReset() {
    reset(constants::kFullMask);
    resonance_ = 0.0f;
    drive_ = 0.0f;
    post_multiply_ = 0.0f;
  }

  /**
   * @brief Processes the audio block through the diode filter.
   * @param num_samples The number of samples to process.
   *
   * This method applies the filter configuration, handles any necessary resets, and then runs the
   * main filter loop. The output is written to the Processor's output buffer.
   */
  void DiodeFilter::process(int num_samples) {
    VITAL_ASSERT(inputMatchesBufferSize(kAudio));

    // Cache internal parameters
    poly_float current_resonance = resonance_;
    poly_float current_drive = drive_;
    poly_float current_post_multiply = post_multiply_;
    poly_float current_high_pass_ratio = high_pass_ratio_;
    poly_float current_high_pass_amount = high_pass_amount_;

    // Fetch filter settings and apply them
    filter_state_.loadSettings(this);
    setupFilter(filter_state_);

    // Check if any voices need resetting
    poly_mask reset_mask = getResetMask(kReset);
    if (reset_mask.anyMask()) {
      reset(reset_mask);

      current_resonance =
        utils::maskLoad(current_resonance, resonance_, reset_mask);
      current_drive =
        utils::maskLoad(current_drive, drive_, reset_mask);
      current_post_multiply =
        utils::maskLoad(current_post_multiply, post_multiply_, reset_mask);
      current_high_pass_ratio =
        utils::maskLoad(current_high_pass_ratio, high_pass_ratio_, reset_mask);
      current_high_pass_amount =
        utils::maskLoad(current_high_pass_amount, high_pass_amount_, reset_mask);
    }

    // Calculate per-sample increments for parameter smoothing
    mono_float tick_increment = 1.0f / num_samples;
    poly_float delta_resonance = (resonance_ - current_resonance) * tick_increment;
    poly_float delta_drive = (drive_ - current_drive) * tick_increment;
    poly_float delta_post_multiply = (post_multiply_ - current_post_multiply) * tick_increment;
    poly_float delta_high_pass_ratio = (high_pass_ratio_ - current_high_pass_ratio) * tick_increment;
    poly_float delta_high_pass_amount = (high_pass_amount_ - current_high_pass_amount) * tick_increment;

    // Get buffers
    const poly_float* audio_in = input(kAudio)->source->buffer;
    poly_float* audio_out = output()->buffer;

    // Retrieve the coefficient lookup table and MIDI note-based filter control
    const CoefficientLookup* coefficient_lookup = getCoefficientLookup();
    const poly_float* midi_cutoff_buffer = filter_state_.midi_cutoff_buffer;
    poly_float base_midi = midi_cutoff_buffer[num_samples - 1];
    poly_float base_frequency =
      utils::midiNoteToFrequency(base_midi) * (1.0f / getSampleRate());
    poly_float high_pass_frequency_ratio =
      kHighPassFrequency * (1.0f / getSampleRate());
    poly_float high_pass_feedback_coefficient =
      coefficient_lookup->cubicLookup(high_pass_frequency_ratio);

    // Main processing loop
    for (int i = 0; i < num_samples; ++i) {
      poly_float midi_delta = midi_cutoff_buffer[i] - base_midi;
      // Calculate the current sample's frequency from MIDI
      poly_float frequency =
        utils::min(base_frequency * futils::midiOffsetToRatio(midi_delta), 1.0f);
      poly_float coefficient = coefficient_lookup->cubicLookup(frequency);

      // Smoothly update parameters
      current_resonance += delta_resonance;
      current_drive += delta_drive;
      current_post_multiply += delta_post_multiply;
      current_high_pass_ratio += delta_high_pass_ratio;
      current_high_pass_amount += delta_high_pass_amount;

      // Apply the diode filter algorithm per sample
      tick(audio_in[i], coefficient, current_high_pass_ratio, current_high_pass_amount,
           high_pass_feedback_coefficient, current_resonance, current_drive);

      // Write output (final stage multiplied by the post_multiply factor)
      audio_out[i] = stage4_.getCurrentState() * current_post_multiply;
    }
  }

  /**
   * @brief Updates internal filter parameters based on the provided FilterState.
   * @param filter_state The FilterState struct that holds parameters like resonance, drive, and style.
   *
   * This method calculates resonance, drive, and high-pass ratios based on the selected style
   * (e.g., 12dB vs. 24dB modes) and user-defined settings.
   */
  void DiodeFilter::setupFilter(const FilterState& filter_state) {
    static constexpr float kHighPassStart = -9.0f;
    static constexpr float kHighPassEnd   = -1.0f;
    static constexpr float kHighPassRange = kHighPassEnd - kHighPassStart;

    // Compute resonance (Q)
    poly_float resonance_percent = utils::clamp(filter_state.resonance_percent, 0.0f, 1.0f);
    // Emphasize higher resonance values
    resonance_percent *= resonance_percent * resonance_percent;

    resonance_ = utils::interpolate(kMinResonance, kMaxResonance, resonance_percent);

    // Compute drive (scales with resonance)
    drive_ = (resonance_ * 0.5f + 1.0f) * filter_state.drive;

    // Post-multiply factor compensates for changes in volume due to drive
    post_multiply_ = poly_float(1.0f) / utils::sqrt(filter_state.drive);

    // Compute the high-pass ratio and amount depending on the style
    poly_float blend_amount = filter_state.pass_blend * 0.5f;

    if (filter_state.style == k12Db) {
      high_pass_ratio_ = futils::exp2(kHighPassEnd);
      high_pass_amount_ = blend_amount * blend_amount;
    }
    else {
      high_pass_ratio_ = futils::exp2(blend_amount * kHighPassRange + kHighPassStart);
      high_pass_amount_ = 1.0f;
    }
  }

  /**
   * @brief Processes a single sample through the diode ladder filter stages.
   * @param audio_in The incoming audio sample.
   * @param coefficient The main low-pass filter coefficient.
   * @param high_pass_ratio The ratio used for high-pass filtering.
   * @param high_pass_amount The blend amount for the high-pass stage.
   * @param high_pass_feedback_coefficient The coefficient used for feedback filtering in the high-pass stage.
   * @param resonance The current resonance (Q) value.
   * @param drive The current drive setting.
   *
   * This function implements the multi-stage diode ladder filter flow, including high-pass
   * filtering, resonant feedback, and nonlinear saturation in various stages.
   */
  force_inline void DiodeFilter::tick(poly_float audio_in,
                                      poly_float coefficient,
                                      poly_float high_pass_ratio,
                                      poly_float high_pass_amount,
                                      poly_float high_pass_feedback_coefficient,
                                      poly_float resonance,
                                      poly_float drive) {
    // Compute high-pass filter coefficients
    poly_float high_pass_coefficient = coefficient * high_pass_ratio;
    poly_float high_pass_coefficient2 = high_pass_coefficient * 2.0f;
    poly_float high_pass_coefficient_squared = high_pass_coefficient * high_pass_coefficient;
    poly_float high_pass_coefficient_diff = high_pass_coefficient_squared - high_pass_coefficient;
    poly_float high_pass_feedback_mult = high_pass_coefficient2 - high_pass_coefficient_squared - 1.0f;
    poly_float high_pass_normalizer = poly_float(1.0f) / (high_pass_coefficient_diff + 1.0f);

    poly_float high_pass_mult_stage2 = -high_pass_coefficient + 1.0f;
    poly_float high_pass_feedback =
      high_pass_feedback_mult * high_pass_1_.getNextState() +
      high_pass_mult_stage2 * high_pass_2_.getNextState();

    // Stage 1: high-pass input
    poly_float high_pass_input = (audio_in - high_pass_feedback) * high_pass_normalizer;

    // Tick through the high-pass filters
    poly_float high_pass_1_out = high_pass_1_.tickBasic(high_pass_input, high_pass_coefficient);
    poly_float high_pass_2_out = high_pass_2_.tickBasic(high_pass_1_out, high_pass_coefficient);

    // Sum to get the high-pass output
    poly_float high_pass_out = high_pass_input - high_pass_1_out * 2.0f + high_pass_2_out;

    // Blend the high-pass output with the original input
    high_pass_out = utils::interpolate(audio_in, high_pass_out, high_pass_amount);

    // Prepare for main diode ladder stages
    poly_float filter_state = stage4_.getNextSatState();
    poly_float filter_input = (drive * high_pass_out - resonance * filter_state) * 0.5f;

    // Nonlinear saturation (tanh)
    poly_float sat_input = futils::tanh(filter_input);

    // Combine with Stage 2 output for feedback
    poly_float feedback_input = sat_input + stage2_.getNextSatState();

    // Tick feedback through its high-pass filter
    poly_float feedback = high_pass_feedback_.tickBasic(feedback_input, high_pass_feedback_coefficient);

    // Cascade through each stage
    stage1_.tick(feedback_input - feedback, coefficient);
    stage2_.tick((stage1_.getCurrentState() + stage3_.getNextSatState()) * 0.5f, coefficient);
    stage3_.tick((stage2_.getCurrentState() + stage4_.getNextSatState()) * 0.5f, coefficient);

    // Final stage with clamping saturation
    stage4_.tick(stage3_.getCurrentState(), coefficient);
  }

} // namespace vital
