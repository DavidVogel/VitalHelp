#include "ladder_filter.h"

#include "futils.h"

namespace vital {

  /**
   * @brief Constructs the LadderFilter and performs a hard reset.
   */
  LadderFilter::LadderFilter() : Processor(LadderFilter::kNumInputs, 1) {
    hardReset();
  }

  /**
   * @brief Resets the internal states of each ladder stage for voices specified by the mask.
   * @param reset_mask A bitmask indicating which voices to reset.
   */
  void LadderFilter::reset(poly_mask reset_mask) {
    // Reset the temporary filter input
    filter_input_ = utils::maskLoad(filter_input_, 0.0f, reset_mask);

    // Reset each of the four one-pole stages
    for (int i = 0; i < kNumStages; ++i)
      stages_[i].reset(reset_mask);
  }

  /**
   * @brief Performs a complete reset of all internal states, clearing resonance, drive, etc.
   */
  void LadderFilter::hardReset() {
    reset(constants::kFullMask);
    resonance_ = 0.0f;
    drive_ = 0.0f;
    post_multiply_ = 0.0f;
  }

  /**
   * @brief Processes a block of audio through this ladder filter.
   * @param num_samples Number of samples to process.
   *
   * Applies smoothing to parameter changes (resonance, drive, etc.), updates stage scales,
   * and then calls tick() for each sample in the input buffer.
   */
  void LadderFilter::process(int num_samples) {
    VITAL_ASSERT(inputMatchesBufferSize(kAudio));

    // Cache the current parameters to smooth them over num_samples
    poly_float current_resonance = resonance_;
    poly_float current_drive = drive_;
    poly_float current_post_multiply = post_multiply_;

    // Cache stage scale factors
    poly_float current_stage_scales[kNumStages + 1];
    for (int i = 0; i <= kNumStages; ++i)
      current_stage_scales[i] = stage_scales_[i];

    // Pull in latest filter settings
    filter_state_.loadSettings(this);
    setupFilter(filter_state_);

    // Check if we need to reset (e.g., new note, or parameter ramp events)
    poly_mask reset_mask = getResetMask(kReset);
    if (reset_mask.anyMask()) {
      reset(reset_mask);

      // Reload parameters for the reset voices
      current_resonance = utils::maskLoad(current_resonance, resonance_, reset_mask);
      current_drive = utils::maskLoad(current_drive, drive_, reset_mask);
      current_post_multiply = utils::maskLoad(current_post_multiply, post_multiply_, reset_mask);
      for (int i = 0; i <= kNumStages; ++i)
        current_stage_scales[i] = utils::maskLoad(current_stage_scales[i], stage_scales_[i], reset_mask);
    }

    // Compute incremental changes for smooth parameter transitions
    mono_float tick_increment = 1.0f / num_samples;
    poly_float delta_resonance = (resonance_ - current_resonance) * tick_increment;
    poly_float delta_drive = (drive_ - current_drive) * tick_increment;
    poly_float delta_post_multiply = (post_multiply_ - current_post_multiply) * tick_increment;

    poly_float delta_stage_scales[kNumStages + 1];
    for (int i = 0; i <= kNumStages; ++i)
      delta_stage_scales[i] = (stage_scales_[i] - current_stage_scales[i]) * tick_increment;

    // Prepare buffers and coefficient lookup
    const poly_float* audio_in = input(kAudio)->source->buffer;
    poly_float* audio_out = output()->buffer;
    const CoefficientLookup* coefficient_lookup = getCoefficientLookup();
    const poly_float* midi_cutoff_buffer = filter_state_.midi_cutoff_buffer;

    // Pre-calculate frequency-related constants
    poly_float base_midi = midi_cutoff_buffer[num_samples - 1];
    poly_float base_frequency = utils::midiNoteToFrequency(base_midi) * (1.0f / getSampleRate());
    poly_float max_frequency = kMaxCutoff / getSampleRate();

    // Process each sample
    for (int i = 0; i < num_samples; ++i) {
      // Compute current cutoff from MIDI pitch
      poly_float midi_delta = midi_cutoff_buffer[i] - base_midi;
      poly_float frequency = utils::min(base_frequency * futils::midiOffsetToRatio(midi_delta), max_frequency);
      poly_float coefficient = coefficient_lookup->cubicLookup(frequency);

      // Smoothly update parameters
      current_resonance += delta_resonance;
      current_drive += delta_drive;
      current_post_multiply += delta_post_multiply;
      for (int stage = 0; stage <= kNumStages; ++stage)
        current_stage_scales[stage] += delta_stage_scales[stage];

      // Process one sample through the ladder filter
      tick(audio_in[i], coefficient, current_resonance, current_drive);

      // Sum up all stage outputs, each scaled appropriately
      poly_float total = current_stage_scales[0] * filter_input_;
      for (int stage = 0; stage < kNumStages; ++stage)
        total += current_stage_scales[stage + 1] * stages_[stage].getCurrentState();

      // Multiply the final sum by the post-multiply factor and write to output
      audio_out[i] = total * current_post_multiply;
    }
  }

  /**
   * @brief Configures filter parameters (resonance, drive, stage scales) from the given FilterState.
   * @param filter_state Object containing style, pass_blend, drive, resonance_percent, etc.
   */
  void LadderFilter::setupFilter(const FilterState& filter_state) {
    // Convert user-specified resonance percent to a local resonance value
    poly_float resonance_percent = utils::clamp(filter_state.resonance_percent, 0.0f, 1.0f);
    poly_float resonance_adjust = resonance_percent;
    if (filter_state.style) {
      // Optional alternative style: sine-based scaling
      resonance_adjust = utils::sin(resonance_percent * (0.5f * kPi));
    }

    // Interpolate between minimum and maximum resonance
    resonance_ = utils::interpolate(kMinResonance, kMaxResonance, resonance_adjust);
    // Boost resonance further based on drive
    resonance_ += filter_state.drive_percent * filter_state.resonance_percent * kDriveResonanceBoost;

    // Adjust stage scales (low-pass, high-pass, etc.) based on style
    setStageScales(filter_state);
  }

  /**
   * @brief Sets the internal stage_scales_ array based on the filter style and pass blends.
   * @param filter_state Filter parameters to use for computing stage scales.
   *
   * The ladder filter’s output can be a combination of multiple slopes: 12 dB, 24 dB, band-pass,
   * high-pass, etc. This method computes how each stage’s output is weighted in the final mix.
   */
  void LadderFilter::setStageScales(const FilterState& filter_state) {
    // Polynomial coefficients for different filter slopes (biquad expansions, etc.)
    static const mono_float low_pass24[kNumStages + 1]   = { 0.0f,  0.0f,  0.0f,   0.0f,  1.0f };
    static const mono_float band_pass24[kNumStages + 1]  = { 0.0f,  0.0f, -1.0f,  2.0f, -1.0f };
    static const mono_float high_pass24[kNumStages + 1]  = { 1.0f, -4.0f,  6.0f, -4.0f,  1.0f };
    static const mono_float low_pass12[kNumStages + 1]   = { 0.0f,  0.0f,  1.0f,  0.0f,  0.0f };
    static const mono_float band_pass12[kNumStages + 1]  = { 0.0f,  1.0f, -1.0f,  0.0f,  0.0f };
    static const mono_float high_pass12[kNumStages + 1]  = { 1.0f, -2.0f,  1.0f,  0.0f,  0.0f };

    // For pass blend, map to -1..1
    poly_float blend = utils::clamp(filter_state.pass_blend - 1.0f, -1.0f, 1.0f);
    // band_pass is the sqrt(1 - blend^2) portion of the circle
    poly_float band_pass = utils::sqrt(-blend * blend + 1.0f);

    // For partial crossfade between low-pass and high-pass
    poly_mask blend_mask = poly_float::lessThan(blend, 0.0f);
    poly_float low_pass = (-blend) & blend_mask;       // Active if blend < 0
    poly_float high_pass = blend & ~blend_mask;        // Active if blend > 0

    // Drive and resonance scaling
    poly_float resonance_percent = utils::clamp(filter_state.resonance_percent, 0.0f, 1.0f);
    poly_float drive_mult = resonance_percent + 1.0f;
    if (filter_state.style)
      drive_mult = utils::sin(resonance_percent) + 1.0f;

    poly_float resonance_scale = utils::interpolate(drive_mult, 1.0f, high_pass);
    drive_ = filter_state.drive * resonance_scale;

    // A factor used to adjust volume after applying drive
    post_multiply_ = poly_float(1.0f)
                     / utils::sqrt((filter_state.drive - 1.0f) * 0.5f + 1.0f);

    // Compute the filter’s output mixing (12 dB or 24 dB, etc.)
    if (filter_state.style == k12Db) {
      for (int i = 0; i <= kNumStages; ++i)
        stage_scales_[i] = (low_pass * low_pass12[i]
                            + band_pass * band_pass12[i]
                            + high_pass * high_pass12[i]);
    }
    else if (filter_state.style == k24Db) {
      // A variation used for a 24 dB slope
      band_pass = -poly_float::abs(blend) + 1.0f;
      post_multiply_ = poly_float(1.0f)
                       / utils::sqrt((filter_state.drive - 1.0f) * 0.25f + 1.0f);

      for (int i = 0; i <= kNumStages; ++i)
        stage_scales_[i] = (low_pass * low_pass24[i]
                            + band_pass * band_pass24[i]
                            + high_pass * high_pass24[i]);
    }
    else if (filter_state.style == kDualNotchBand) {
      // A 'dual notch band' style of ladder mixing
      drive_ = filter_state.drive;  // No scaling for drive in this style
      poly_float low_pass_fade = utils::min(blend + 1.0f, 1.0f);
      poly_float high_pass_fade = utils::min(-blend + 1.0f, 1.0f);

      stage_scales_[0] = low_pass_fade;
      stage_scales_[1] = low_pass_fade * -4.0f;
      stage_scales_[2] = high_pass_fade * 4.0f + low_pass_fade * 8.0f;
      stage_scales_[3] = high_pass_fade * -8.0f - low_pass_fade * 8.0f;
      stage_scales_[4] = high_pass_fade * 4.0f + low_pass_fade * 4.0f;
    }
    else if (filter_state.style == kNotchPassSwap) {
      post_multiply_ = poly_float(1.0f)
                       / utils::sqrt((filter_state.drive - 1.0f) * 0.5f + 1.0f);

      poly_float low_pass_fade = utils::min(blend + 1.0f, 1.0f);
      poly_float low_pass_fade2 = low_pass_fade * low_pass_fade;
      poly_float high_pass_fade = utils::min(-blend + 1.0f, 1.0f);
      poly_float high_pass_fade2 = high_pass_fade * high_pass_fade;
      poly_float low_high_pass_fade = low_pass_fade * high_pass_fade;

      stage_scales_[0] = low_pass_fade2;
      stage_scales_[1] = low_pass_fade2 * -4.0f;
      stage_scales_[2] = low_pass_fade2 * 6.0f + low_high_pass_fade * 2.0f;
      stage_scales_[3] = low_pass_fade2 * -4.0f - low_high_pass_fade * 4.0f;
      stage_scales_[4] = low_pass_fade2 + high_pass_fade2 + low_high_pass_fade * 2.0f;
    }
    else if (filter_state.style == kBandPeakNotch) {
      // Another specialized style that uses a band/peak/notch configuration
      poly_float drive_t = poly_float::min(-blend + 1.0f, 1.0f);
      drive_ = utils::interpolate(filter_state.drive, drive_, drive_t);

      poly_float drive_inv_t = -drive_t + 1.0f;
      poly_float mult = utils::sqrt((drive_inv_t * drive_inv_t) * 0.5f + 0.5f);
      poly_float peak_band_value = -utils::max(-blend, 0.0f);
      poly_float low_high = mult * (peak_band_value + 1.0f);
      poly_float band = mult * (peak_band_value - blend + 1.0f) * 2.0f;

      for (int i = 0; i <= kNumStages; ++i)
        stage_scales_[i] = (low_high * low_pass12[i]
                            + band * band_pass12[i]
                            + low_high * high_pass12[i]);
    }
  }

  /**
   * @brief Processes a single sample of audio through the 4-pole ladder stages.
   * @param audio_in The input sample.
   * @param coefficient The filter coefficient controlling the cutoff.
   * @param resonance The current resonance value to be applied.
   * @param drive The amount of drive (input gain) to apply before the filter stages.
   */
  force_inline void LadderFilter::tick(poly_float audio_in, poly_float coefficient,
                                       poly_float resonance, poly_float drive) {
    // Multiply coefficient by a fixed tuning factor to better match classic ladder response
    poly_float g1 = coefficient * kResonanceTuning;
    poly_float g2 = g1 * g1;
    poly_float g3 = g1 * g2;

    // The final stage's output from the previous sample is fed back through g1..g3
    poly_float filter_state1 = utils::mulAdd(stages_[3].getNextSatState(), g1, stages_[2].getNextSatState());
    poly_float filter_state2 = utils::mulAdd(filter_state1, g2, stages_[1].getNextSatState());
    poly_float filter_state = utils::mulAdd(filter_state2, g3, stages_[0].getNextSatState());

    // Combine input (with drive) and negative feedback from the final stage
    poly_float filter_input = (audio_in * drive - resonance * filter_state);

    // Use a nonlinear function (tanh) for mild saturation
    filter_input_ = futils::tanh(filter_input);

    // Pass through each stage, each employing algebraic saturation
    poly_float stage_out = stages_[0].tick(filter_input_, coefficient);
    stage_out = stages_[1].tick(stage_out, coefficient);
    stage_out = stages_[2].tick(stage_out, coefficient);
    stages_[3].tick(stage_out, coefficient); // final stage in the pipeline
  }

} // namespace vital
