#include "sallen_key_filter.h"
#include "futils.h"

namespace vital {

  /**
   * @brief Constructs a SallenKeyFilter and performs an initial hard reset.
   */
  SallenKeyFilter::SallenKeyFilter() : Processor(SallenKeyFilter::kNumInputs, 1) {
    hardReset();
  }

  /**
   * @brief Resets the internal states of the filter (one-pole stages, etc.) for the specified voices.
   * @param reset_mask A bitmask indicating which voices to reset.
   */
  void SallenKeyFilter::reset(poly_mask reset_mask) {
    stage1_input_ = utils::maskLoad(stage1_input_, 0.0f, reset_mask);

    pre_stage1_.reset(reset_mask);
    pre_stage2_.reset(reset_mask);
    stage1_.reset(reset_mask);
    stage2_.reset(reset_mask);
  }

  /**
   * @brief Performs a hard reset of all parameters and filter states to default values.
   */
  void SallenKeyFilter::hardReset() {
    reset(constants::kFullMask);
    resonance_ = 0.0f;
    drive_ = 0.0f;
    post_multiply_ = 0.0f;
    low_pass_amount_ = 0.0f;
    band_pass_amount_ = 0.0f;
    high_pass_amount_ = 0.0f;
  }

  /**
   * @brief Main process function. Reads audio from the input buffer, applies filtering, and writes to the output.
   * @param num_samples Number of samples to process.
   */
  void SallenKeyFilter::process(int num_samples) {
    VITAL_ASSERT(inputMatchesBufferSize(kAudio));
    processWithInput(input(kAudio)->source->buffer, num_samples);
  }

  /**
   * @brief Processes an external input buffer of audio using current filter parameters.
   * @param audio_in Pointer to the incoming audio samples.
   * @param num_samples Number of samples to process.
   */
  void SallenKeyFilter::processWithInput(const poly_float* audio_in, int num_samples) {
    // Cache parameters before we attempt smoothing
    poly_float current_resonance = resonance_;
    poly_float current_drive = drive_;
    poly_float current_post_multiply = post_multiply_;
    poly_float current_low = low_pass_amount_;
    poly_float current_band = band_pass_amount_;
    poly_float current_high = high_pass_amount_;

    // Reload filter state (e.g., if the user changed settings in real time)
    filter_state_.loadSettings(this);
    setupFilter(filter_state_);

    // Check for any voices needing reset
    poly_mask reset_mask = getResetMask(kReset);
    if (reset_mask.anyMask()) {
      reset(reset_mask);

      // Reload those parameters for the reset voices
      current_resonance = utils::maskLoad(current_resonance, resonance_, reset_mask);
      current_drive = utils::maskLoad(current_drive, drive_, reset_mask);
      current_post_multiply = utils::maskLoad(current_post_multiply, post_multiply_, reset_mask);
      current_low = utils::maskLoad(current_low, low_pass_amount_, reset_mask);
      current_band = utils::maskLoad(current_band, band_pass_amount_, reset_mask);
      current_high = utils::maskLoad(current_high, high_pass_amount_, reset_mask);
    }

    // Dispatch processing based on style (12 dB, 24 dB, or dual notch)
    if (filter_state_.style == k12Db) {
      process12(audio_in, num_samples, current_resonance, current_drive, current_post_multiply,
                current_low, current_band, current_high);
    }
    else if (filter_state_.style == kDualNotchBand) {
      processDual(audio_in, num_samples, current_resonance, current_drive, current_post_multiply,
                  current_low, current_high);
    }
    else {
      process24(audio_in, num_samples, current_resonance, current_drive, current_post_multiply,
                current_low, current_band, current_high);
    }
  }

  /**
   * @brief Processes audio using a 12 dB (2-pole) Sallen-Key filter configuration.
   * @param audio_in Input audio buffer.
   * @param num_samples Number of samples to process.
   * @param current_resonance Current resonance value to be smoothed over the block.
   * @param current_drive Current drive amount to be smoothed.
   * @param current_post_multiply Normalization factor to be smoothed.
   * @param current_low Current low-pass mix value.
   * @param current_band Current band-pass mix value.
   * @param current_high Current high-pass mix value.
   */
  void SallenKeyFilter::process12(const poly_float* audio_in, int num_samples,
                                  poly_float current_resonance,
                                  poly_float current_drive, poly_float current_post_multiply,
                                  poly_float current_low, poly_float current_band, poly_float current_high) {
    mono_float tick_increment = 1.0f / num_samples;
    // Compute deltas for parameter smoothing
    poly_float delta_resonance = (resonance_ - current_resonance) * tick_increment;
    poly_float delta_drive = (drive_ - current_drive) * tick_increment;
    poly_float delta_post_multiply = (post_multiply_ - current_post_multiply) * tick_increment;
    poly_float delta_low = (low_pass_amount_ - current_low) * tick_increment;
    poly_float delta_band = (band_pass_amount_ - current_band) * tick_increment;
    poly_float delta_high = (high_pass_amount_ - current_high) * tick_increment;

    // Retrieve coefficient lookup and MIDI-based cutoff data
    const CoefficientLookup* coefficient_lookup = getCoefficientLookup();
    const poly_float* midi_cutoff_buffer = filter_state_.midi_cutoff_buffer;
    poly_float base_midi = midi_cutoff_buffer[num_samples - 1];
    poly_float base_frequency =
      utils::midiNoteToFrequency(base_midi) * (1.0f / getSampleRate());

    poly_float* audio_out = output()->buffer;

    // Main loop
    for (int i = 0; i < num_samples; ++i) {
      // Calculate the local cutoff coefficient
      poly_float midi_delta = midi_cutoff_buffer[i] - base_midi;
      poly_float frequency =
        utils::min(base_frequency * futils::midiOffsetToRatio(midi_delta), 1.0f);
      poly_float coefficient = coefficient_lookup->cubicLookup(frequency);

      // Update parameters
      current_resonance += delta_resonance;
      current_drive += delta_drive;
      current_post_multiply += delta_post_multiply;
      current_low += delta_low;
      current_band += delta_band;
      current_high += delta_high;

      // Compute resonance and other Sallen-Key factors
      poly_float coefficient2 = coefficient * 2.0f;
      poly_float resonance = tuneResonance(current_resonance, coefficient2);
      poly_float stage1_feedback_mult = coefficient2 - coefficient * coefficient - 1.0f;
      poly_float normalizer =
        poly_float(1.0f) / (resonance * (coefficient * coefficient - coefficient) + 1.0f);

      // Tick the filter (2-pole version)
      tick(audio_in[i], coefficient, resonance, stage1_feedback_mult, current_drive, normalizer);

      // 2-pole Sallen-Key output retrieval
      poly_float stage2_input = stage1_.getCurrentState();
      poly_float low_pass = stage2_.getCurrentState();
      poly_float band_pass = stage2_input - low_pass;
      poly_float high_pass = stage1_input_ - stage2_input - band_pass;

      // Mix the final output based on low, band, and high amounts
      poly_float low = current_low * low_pass;
      poly_float band_low = utils::mulAdd(low, current_band, band_pass);
      audio_out[i] =
        utils::mulAdd(band_low, current_high, high_pass) * current_post_multiply;

      VITAL_ASSERT(utils::isFinite(audio_out[i]));
    }
  }

  /**
   * @brief Processes audio using a 24 dB (4-pole) Sallen-Key filter configuration.
   * @param audio_in Input audio buffer.
   * @param num_samples Number of samples to process.
   * @param current_resonance Current resonance value.
   * @param current_drive Current drive amount.
   * @param current_post_multiply Normalization factor after drive.
   * @param current_low Current low-pass mix.
   * @param current_band Current band-pass mix.
   * @param current_high Current high-pass mix.
   */
  void SallenKeyFilter::process24(const poly_float* audio_in, int num_samples,
                                  poly_float current_resonance,
                                  poly_float current_drive, poly_float current_post_multiply,
                                  poly_float current_low, poly_float current_band, poly_float current_high) {
    mono_float tick_increment = 1.0f / num_samples;
    // Compute deltas for smoothing
    poly_float delta_resonance = (resonance_ - current_resonance) * tick_increment;
    poly_float delta_drive = (drive_ - current_drive) * tick_increment;
    poly_float delta_post_multiply = (post_multiply_ - current_post_multiply) * tick_increment;
    poly_float delta_low = (low_pass_amount_ - current_low) * tick_increment;
    poly_float delta_band = (band_pass_amount_ - current_band) * tick_increment;
    poly_float delta_high = (high_pass_amount_ - current_high) * tick_increment;

    const CoefficientLookup* coefficient_lookup = getCoefficientLookup();
    const poly_float* midi_cutoff_buffer = filter_state_.midi_cutoff_buffer;
    poly_float base_midi = midi_cutoff_buffer[num_samples - 1];
    poly_float base_frequency =
      utils::midiNoteToFrequency(base_midi) * (1.0f / getSampleRate());

    poly_float* audio_out = output()->buffer;

    // Main loop
    for (int i = 0; i < num_samples; ++i) {
      poly_float midi_delta = midi_cutoff_buffer[i] - base_midi;
      poly_float frequency =
        utils::min(base_frequency * futils::midiOffsetToRatio(midi_delta), 1.0f);
      poly_float coefficient = coefficient_lookup->cubicLookup(frequency);

      current_resonance += delta_resonance;
      current_drive += delta_drive;
      current_post_multiply += delta_post_multiply;
      current_low += delta_low;
      current_band += delta_band;
      current_high += delta_high;

      poly_float coefficient2 = coefficient * 2.0f;
      poly_float coefficient_squared = coefficient * coefficient;
      poly_float resonance = tuneResonance(current_resonance, coefficient2);

      // Compute specialized factors for 24 dB Sallen-Key
      poly_float stage1_feedback_mult = coefficient2 - coefficient_squared - 1.0f;
      poly_float pre_normalizer =
        poly_float(1.0f) / ((coefficient_squared - coefficient) + 1.0f);
      poly_float normalizer =
        poly_float(1.0f) / (resonance * (coefficient_squared - coefficient) + 1.0f);

      // Tick the 4-pole version
      tick24(audio_in[i], coefficient, resonance, stage1_feedback_mult, current_drive,
             pre_normalizer, normalizer, current_low, current_band, current_high);

      // Extract final 4-pole output
      poly_float stage2_input = stage1_.getCurrentState();
      poly_float low_pass = stage2_.getCurrentState();
      poly_float band_pass = stage2_input - low_pass;
      poly_float high_pass = stage1_input_ - stage2_input - band_pass;

      // Combine low, band, and high outputs
      poly_float low = current_low * low_pass;
      poly_float band_low = utils::mulAdd(low, current_band, band_pass);
      audio_out[i] =
        utils::mulAdd(band_low, current_high, high_pass) * current_post_multiply;

      VITAL_ASSERT(utils::isFinite(audio_out[i]));
    }
  }

  /**
   * @brief Processes audio with a dual-notch filter style, blending low/high outputs differently.
   * @param audio_in Pointer to the input audio buffer.
   * @param num_samples Number of samples to process.
   * @param current_resonance Current resonance value.
   * @param current_drive Current drive amount.
   * @param current_post_multiply Post-normalization factor.
   * @param current_low Current low-pass mix amount.
   * @param current_high Current high-pass mix amount.
   */
  void SallenKeyFilter::processDual(const poly_float* audio_in, int num_samples,
                                    poly_float current_resonance,
                                    poly_float current_drive, poly_float current_post_multiply,
                                    poly_float current_low, poly_float current_high) {
    mono_float tick_increment = 1.0f / num_samples;
    poly_float delta_resonance = (resonance_ - current_resonance) * tick_increment;
    poly_float delta_drive = (drive_ - current_drive) * tick_increment;
    poly_float delta_post_multiply = (post_multiply_ - current_post_multiply) * tick_increment;
    poly_float delta_low = (low_pass_amount_ - current_low) * tick_increment;
    poly_float delta_high = (high_pass_amount_ - current_high) * tick_increment;

    const CoefficientLookup* coefficient_lookup = getCoefficientLookup();
    const poly_float* midi_cutoff_buffer = filter_state_.midi_cutoff_buffer;
    poly_float base_midi = midi_cutoff_buffer[num_samples - 1];
    poly_float base_frequency =
      utils::midiNoteToFrequency(base_midi) * (1.0f / getSampleRate());

    poly_float* audio_out = output()->buffer;

    for (int i = 0; i < num_samples; ++i) {
      poly_float midi_delta = midi_cutoff_buffer[i] - base_midi;
      poly_float frequency =
        utils::min(base_frequency * futils::midiOffsetToRatio(midi_delta), 1.0f);
      poly_float coefficient = coefficient_lookup->cubicLookup(frequency);

      current_resonance += delta_resonance;
      current_drive += delta_drive;
      current_post_multiply += delta_post_multiply;
      current_low += delta_low;
      current_high += delta_high;

      // Prepare Sallen-Key factors
      poly_float coefficient2 = coefficient * 2.0f;
      poly_float coefficient_squared = coefficient * coefficient;
      poly_float resonance = tuneResonance(current_resonance, coefficient2);
      poly_float stage1_feedback_mult = coefficient2 - coefficient_squared - 1.0f;
      poly_float pre_normalizer =
        poly_float(1.0f) / ((coefficient_squared - coefficient) + 1.0f);
      poly_float normalizer =
        poly_float(1.0f) / (resonance * (coefficient_squared - coefficient) + 1.0f);

      // For dual style, reuse tick24 logic with zero band-pass
      tick24(audio_in[i], coefficient, resonance, stage1_feedback_mult, current_drive,
             pre_normalizer, normalizer, current_low, 0.0f, current_high);

      poly_float stage2_input = stage1_.getCurrentState();
      poly_float low_pass = stage2_.getCurrentState();
      // This arrangement merges portions of low and high differently for the dual-notch approach
      poly_float high_pass = stage1_input_ - stage2_input - stage2_input + low_pass;

      // Mix final output (blend of low and high)
      poly_float low = current_high * low_pass;
      audio_out[i] =
        utils::mulAdd(low, current_low, high_pass) * current_post_multiply;

      VITAL_ASSERT(utils::isFinite(audio_out[i]));
    }
  }

  /**
   * @brief Sets up filter parameters based on the provided FilterState (cutoff, resonance, drive, etc.).
   * @param filter_state The new filter settings.
   */
  void SallenKeyFilter::setupFilter(const FilterState& filter_state) {
    // Convert MIDI cutoff to frequency
    cutoff_ = utils::midiNoteToFrequency(filter_state.midi_cutoff);
    float min_nyquist = getSampleRate() * kMinNyquistMult;
    cutoff_ = utils::clamp(cutoff_, kMinCutoff, min_nyquist);

    // Compute resonance from the input parameter
    poly_float resonance_percent =
      utils::clamp(filter_state.resonance_percent, 0.0f, 1.0f);
    // Use sqrt to skew resonance distribution
    resonance_percent = utils::sqrt(resonance_percent);

    resonance_ =
      utils::interpolate(kMinResonance, kMaxResonance, resonance_percent);
    // Additional boost based on drive
    resonance_ += filter_state.drive_percent
                  * filter_state.resonance_percent
                  * kDriveResonanceBoost;

    // pass_blend mapped to -1..1
    poly_float blend =
      utils::clamp(filter_state.pass_blend - 1.0f, -1.0f, 1.0f);

    // Scale drive by the squared resonance factor
    poly_float resonance_scale =
      resonance_percent * resonance_percent * 2.0f + 1.0f;
    drive_ = poly_float(filter_state.drive) / resonance_scale;

    // Handle style-specific parameter routing
    if (filter_state.style == kDualNotchBand) {
      // Specialized blending for dual-notch
      poly_float t = blend * 0.5f + 0.5f;
      poly_float drive_t = poly_float::min(-blend + 1.0f, 1.0f);
      poly_float drive_mult = -t + 2.0f;
      drive_ = utils::interpolate(filter_state.drive, drive_ * drive_mult, drive_t);

      low_pass_amount_ = t;
      band_pass_amount_ = 0.0f;
      high_pass_amount_ = 1.0f;
    }
    else if (filter_state.style == kNotchPassSwap) {
      // Blend for a notch/pass swap style
      poly_float drive_t = poly_float::abs(blend);
      drive_ = utils::interpolate(filter_state.drive, drive_, drive_t);

      low_pass_amount_ = utils::min(-blend + 1.0f, 1.0f);
      band_pass_amount_ = 0.0f;
      high_pass_amount_ = utils::min(blend + 1.0f, 1.0f);
    }
    else if (filter_state.style == kBandPeakNotch) {
      // Band/peak/notch style
      poly_float drive_t = poly_float::min(-blend + 1.0f, 1.0f);
      drive_ = utils::interpolate(filter_state.drive, drive_, drive_t);

      poly_float drive_inv_t = -drive_t + 1.0f;
      poly_float mult =
        utils::sqrt((drive_inv_t * drive_inv_t) * 0.5f + 0.5f);
      poly_float peak_band_value = -utils::max(-blend, 0.0f);
      low_pass_amount_ = mult * (peak_band_value + 1.0f);
      band_pass_amount_ =
        mult * (peak_band_value - blend + 1.0f) * 2.0f;
      high_pass_amount_ = low_pass_amount_;
    }
    else {
      // Default 12 dB or 24 dB blending
      band_pass_amount_ = utils::sqrt(-blend * blend + 1.0f);
      poly_mask blend_mask = poly_float::lessThan(blend, 0.0f);
      low_pass_amount_ = (-blend) & blend_mask;
      high_pass_amount_ = blend & ~blend_mask;
    }

    // Post-multiply factor
    post_multiply_ =
      poly_float(1.0f) / utils::sqrt(resonance_scale * drive_);
  }

  /**
   * @brief A specialized single-sample tick for the 4-pole (24 dB) Sallen-Key filter.
   * @param audio_in The current input sample.
   * @param coefficient The cutoff-related filter coefficient.
   * @param resonance Filter resonance.
   * @param stage1_feedback_mult Computed feedback factor for the first stage.
   * @param drive Scales the input sample.
   * @param pre_normalizer Partial normalization for the pre-stages.
   * @param normalizer Additional normalization for the main filter stage.
   * @param low Low-pass blend amount.
   * @param band Band-pass blend amount.
   * @param high High-pass blend amount.
   */
  force_inline void SallenKeyFilter::tick24(poly_float audio_in, poly_float coefficient,
                                            poly_float resonance, poly_float stage1_feedback_mult, poly_float drive,
                                            poly_float pre_normalizer, poly_float normalizer,
                                            poly_float low, poly_float band, poly_float high) {
    // Preliminary stage feedback using the pre_stage1_ and pre_stage2_ filters
    poly_float mult_stage2 = -coefficient + 1.0f;
    poly_float feedback =
      utils::mulAdd(stage1_feedback_mult * pre_stage1_.getNextState(),
                    mult_stage2, pre_stage2_.getNextState());

    // Pre-stage input
    poly_float stage1_input = (audio_in - feedback) * pre_normalizer;

    // Pass through two one-pole filters for preliminary 2 poles
    poly_float stage1_out = pre_stage1_.tickBasic(stage1_input, coefficient);
    poly_float stage2_out = pre_stage2_.tickBasic(stage1_out, coefficient);

    // Temporary band-pass and high-pass from these first 2 poles
    poly_float band_pass_out = stage1_out - stage2_out;
    poly_float high_pass_out = stage1_input - stage1_out - band_pass_out;

    // Combine them based on the user’s low, band, high
    poly_float low_out = low * stage2_out;
    poly_float band_low_out = utils::mulAdd(low_out, band, band_pass_out);
    poly_float audio_out = utils::mulAdd(band_low_out, high, high_pass_out);

    // The second pair of poles uses the main tick() function
    tick(audio_out, coefficient, resonance, stage1_feedback_mult, drive, normalizer);
  }

  /**
   * @brief A generalized single-sample tick function for a 2-pole (or shared logic) Sallen-Key filter.
   * @param audio_in The current input sample.
   * @param coefficient The cutoff-based coefficient.
   * @param resonance Filter resonance (adjusted).
   * @param stage1_feedback_mult A feedback scaling factor from the first stage.
   * @param drive Drive factor to scale the incoming sample.
   * @param normalizer Normalization factor for balancing gain changes.
   */
  force_inline void SallenKeyFilter::tick(poly_float audio_in, poly_float coefficient, poly_float resonance,
                                          poly_float stage1_feedback_mult, poly_float drive, poly_float normalizer) {
    // Compute feedback from the first stage
    poly_float mult_stage2 = -coefficient + 1.0f;
    poly_float feedback =
      utils::mulAdd(stage1_feedback_mult * stage1_.getNextState(),
                    mult_stage2, stage2_.getNextState());

    // Nonlinear saturation (tanh) is used here to manage extremes
    stage1_input_ = futils::tanh((drive * audio_in - resonance * feedback) * normalizer);

    // Pass through two one-pole filters (1st and 2nd stage) for 2-pole effect
    poly_float stage1_out = stage1_.tickBasic(stage1_input_, coefficient);
    stage2_.tickBasic(stage1_out, coefficient);
  }
} // namespace vital
