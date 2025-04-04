#include "digital_svf.h"

#include "futils.h"

namespace vital {
  /**
   * @brief The global SVF coefficient lookup table instance.
   *
   * Initializes a 2048-entry table converting normalized frequency ratios to
   * one-pole filter coefficients using computeSvfOnePoleFilterCoefficient().
   */
  const DigitalSvf::SvfCoefficientLookup DigitalSvf::svf_coefficient_lookup_;

  /**
   * @brief Constructs a DigitalSvf object, resetting its internal states to defaults.
   */
  DigitalSvf::DigitalSvf() : Processor(DigitalSvf::kNumInputs, 1),
                             min_resonance_(kDefaultMinResonance), max_resonance_(kDefaultMaxResonance),
                             basic_(false), drive_compensation_(true) {
    hardReset();
  }

  /**
   * @brief Processes a block of samples by reading from the main audio input,
   *        then calls processWithInput().
   *
   * @param num_samples Number of samples to process.
   */
  void DigitalSvf::process(int num_samples) {
    VITAL_ASSERT(inputMatchesBufferSize(kAudio));
    VITAL_ASSERT(inputMatchesBufferSize(kMidiCutoff));
    processWithInput(input(kAudio)->source->buffer, num_samples);
  }

  /**
   * @brief Detailed logic for processing a block of samples, handling filter style and interpolation.
   *
   * @param audio_in    Pointer to the input audio buffer.
   * @param num_samples Number of samples to process.
   */
  void DigitalSvf::processWithInput(const poly_float* audio_in, int num_samples) {
    // Copy local state for interpolation
    FilterValues blends1 = blends1_;
    FilterValues blends2 = blends2_;
    poly_float current_resonance = resonance_;
    poly_float current_drive = drive_;
    poly_float current_post_multiply = post_multiply_;

    // Load updated filter state parameters
    filter_state_.loadSettings(this);
    setupFilter(filter_state_);

    // Check reset mask
    poly_mask reset_mask = getResetMask(kReset);
    if (reset_mask.anyMask()) {
      reset(reset_mask);
      blends1.reset(reset_mask, blends1_);
      blends2.reset(reset_mask, blends2_);
      current_resonance = utils::maskLoad(current_resonance, resonance_, reset_mask);
      current_drive = utils::maskLoad(current_drive, drive_, reset_mask);
      current_post_multiply = utils::maskLoad(current_post_multiply, post_multiply_, reset_mask);
    }

    // Branch based on filter style
    if (filter_state_.style == kShelving || basic_)
      processBasic12(audio_in, num_samples, current_resonance, current_drive, current_post_multiply, blends1);
    else if (filter_state_.style == kDualNotchBand) {
      processDual(audio_in, num_samples, current_resonance, current_drive, current_post_multiply, blends1, blends2);
    }
    else if (filter_state_.style == k12Db)
      process12(audio_in, num_samples, current_resonance, current_drive, current_post_multiply, blends1);
    else
      process24(audio_in, num_samples, current_resonance, current_drive, current_post_multiply, blends1);
  }

  void DigitalSvf::process12(const poly_float* audio_in, int num_samples,
                             poly_float current_resonance, poly_float current_drive,
                             poly_float current_post_multiply, FilterValues& blends) {
    mono_float sample_inc = 1.0f / num_samples;
    FilterValues delta_blends = blends.getDelta(blends1_, sample_inc);
    poly_float delta_resonance = (resonance_ - current_resonance) * sample_inc;
    poly_float delta_drive = (drive_ - current_drive) * sample_inc;
    poly_float delta_post_multiply = (post_multiply_ - current_post_multiply) * sample_inc;

    poly_float* audio_out = output()->buffer;
    const SvfCoefficientLookup* coefficient_lookup = getSvfCoefficientLookup();
    const poly_float* midi_cutoff_buffer = filter_state_.midi_cutoff_buffer;
    poly_float base_midi = midi_cutoff_buffer[num_samples - 1];
    poly_float base_frequency = utils::midiNoteToFrequency(base_midi) * (1.0f / getSampleRate());

    for (int i = 0; i < num_samples; ++i) {
      poly_float midi_delta = utils::max(midi_cutoff_buffer[i], 0.0f) - base_midi;
      poly_float frequency = utils::min(base_frequency * futils::midiOffsetToRatio(midi_delta), 1.0f);
      poly_float coefficient = coefficient_lookup->cubicLookup(frequency);

      blends.increment(delta_blends);
      current_resonance += delta_resonance;
      current_drive += delta_drive;
      current_post_multiply += delta_post_multiply;

      audio_out[i] = tick(audio_in[i], coefficient, current_resonance, current_drive, blends) * current_post_multiply;
      VITAL_ASSERT(utils::isFinite(audio_out[i]));
    }
  }

  void DigitalSvf::processBasic12(const poly_float* audio_in, int num_samples,
                                  poly_float current_resonance, poly_float current_drive,
                                  poly_float current_post_multiply, FilterValues& blends) {
    mono_float sample_inc = 1.0f / num_samples;
    FilterValues delta_blends = blends.getDelta(blends1_, sample_inc);
    poly_float delta_resonance = (resonance_ - current_resonance) * sample_inc;
    poly_float delta_drive = (drive_ - current_drive) * sample_inc;
    poly_float delta_post_multiply = (post_multiply_ - current_post_multiply) * sample_inc;

    poly_float* audio_out = output()->buffer;
    const SvfCoefficientLookup* coefficient_lookup = getSvfCoefficientLookup();
    const poly_float* midi_cutoff_buffer = filter_state_.midi_cutoff_buffer;
    poly_float base_midi = midi_cutoff_buffer[num_samples - 1];
    poly_float base_frequency = utils::midiNoteToFrequency(base_midi) * (1.0f / getSampleRate());

    for (int i = 0; i < num_samples; ++i) {
      poly_float midi_delta = midi_cutoff_buffer[i] - base_midi;
      poly_float frequency = utils::min(base_frequency * futils::midiOffsetToRatio(midi_delta), 1.0f);
      poly_float coefficient = coefficient_lookup->cubicLookup(frequency);

      blends.increment(delta_blends);
      current_resonance += delta_resonance;
      current_drive += delta_drive;
      current_post_multiply += delta_post_multiply;

      audio_out[i] = tickBasic(audio_in[i], coefficient, current_resonance, current_drive, blends) * current_post_multiply;
      VITAL_ASSERT(utils::isFinite(audio_out[i]));
    }
  }

  void DigitalSvf::process24(const poly_float* audio_in, int num_samples,
                             poly_float current_resonance, poly_float current_drive,
                             poly_float current_post_multiply, FilterValues& blends) {
    mono_float sample_inc = 1.0f / num_samples;
    FilterValues delta_blends = blends.getDelta(blends1_, sample_inc);
    poly_float delta_resonance = (resonance_ - current_resonance) * sample_inc;
    poly_float delta_drive = (drive_ - current_drive) * sample_inc;
    poly_float delta_post_multiply = (post_multiply_ - current_post_multiply) * sample_inc;

    poly_float* audio_out = output()->buffer;
    const SvfCoefficientLookup* coefficient_lookup = getSvfCoefficientLookup();
    const poly_float* midi_cutoff_buffer = filter_state_.midi_cutoff_buffer;
    poly_float base_midi = midi_cutoff_buffer[num_samples - 1];
    poly_float base_frequency = utils::midiNoteToFrequency(base_midi) * (1.0f / getSampleRate());

    for (int i = 0; i < num_samples; ++i) {
      poly_float midi_delta = midi_cutoff_buffer[i] - base_midi;
      poly_float frequency = utils::min(base_frequency * futils::midiOffsetToRatio(midi_delta), 1.0f);
      poly_float coefficient = coefficient_lookup->cubicLookup(frequency);

      blends.increment(delta_blends);
      current_resonance += delta_resonance;
      current_drive += delta_drive;
      current_post_multiply += delta_post_multiply;

      poly_float result = tick24(audio_in[i], coefficient, current_resonance, current_drive, blends);
      audio_out[i] = result * current_post_multiply;
      VITAL_ASSERT(utils::isFinite(audio_out[i]));
    }
  }

  void DigitalSvf::processBasic24(const poly_float* audio_in, int num_samples,
                                  poly_float current_resonance, poly_float current_drive,
                                  poly_float current_post_multiply, FilterValues& blends) {
    mono_float sample_inc = 1.0f / num_samples;
    FilterValues delta_blends = blends.getDelta(blends1_, sample_inc);
    poly_float delta_resonance = (resonance_ - current_resonance) * sample_inc;
    poly_float delta_drive = (drive_ - current_drive) * sample_inc;
    poly_float delta_post_multiply = (post_multiply_ - current_post_multiply) * sample_inc;

    poly_float* audio_out = output()->buffer;
    const SvfCoefficientLookup* coefficient_lookup = getSvfCoefficientLookup();
    const poly_float* midi_cutoff_buffer = filter_state_.midi_cutoff_buffer;
    poly_float base_midi = midi_cutoff_buffer[num_samples - 1];
    poly_float base_frequency = utils::midiNoteToFrequency(base_midi) * (1.0f / getSampleRate());

    for (int i = 0; i < num_samples; ++i) {
      poly_float midi_delta = midi_cutoff_buffer[i] - base_midi;
      poly_float frequency = utils::min(base_frequency * futils::midiOffsetToRatio(midi_delta), 1.0f);
      poly_float coefficient = coefficient_lookup->cubicLookup(frequency);

      blends.increment(delta_blends);
      current_resonance += delta_resonance;
      current_drive += delta_drive;
      current_post_multiply += delta_post_multiply;

      poly_float result = tickBasic24(audio_in[i], coefficient, current_resonance, current_drive, blends);
      audio_out[i] = result * current_post_multiply;
      VITAL_ASSERT(utils::isFinite(audio_out[i]));
    }
  }

  void DigitalSvf::processDual(const poly_float* audio_in, int num_samples,
                               poly_float current_resonance, poly_float current_drive,
                               poly_float current_post_multiply,
                               FilterValues& blends1, FilterValues& blends2) {
    mono_float sample_inc = 1.0f / num_samples;
    FilterValues delta_blends1 = blends1.getDelta(blends1_, sample_inc);
    FilterValues delta_blends2 = blends2.getDelta(blends2_, sample_inc);
    poly_float delta_resonance = (resonance_ - current_resonance) * sample_inc;
    poly_float delta_drive = (drive_ - current_drive) * sample_inc;
    poly_float delta_post_multiply = (post_multiply_ - current_post_multiply) * sample_inc;

    poly_float* audio_out = output()->buffer;
    const SvfCoefficientLookup* coefficient_lookup = getSvfCoefficientLookup();
    const poly_float* midi_cutoff_buffer = filter_state_.midi_cutoff_buffer;
    poly_float base_midi = midi_cutoff_buffer[num_samples - 1];
    poly_float base_frequency = utils::midiNoteToFrequency(base_midi) * (1.0f / getSampleRate());

    for (int i = 0; i < num_samples; ++i) {
      poly_float midi_delta = midi_cutoff_buffer[i] - base_midi;
      poly_float frequency = utils::min(base_frequency * futils::midiOffsetToRatio(midi_delta), 1.0f);
      poly_float coefficient = coefficient_lookup->cubicLookup(frequency);

      blends1.increment(delta_blends1);
      blends2.increment(delta_blends2);
      current_resonance += delta_resonance;
      current_drive += delta_drive;
      current_post_multiply += delta_post_multiply;

      poly_float result = tickDual(audio_in[i], coefficient, current_resonance, current_drive, blends1, blends2);
      audio_out[i] = result * current_post_multiply;
      VITAL_ASSERT(utils::isFinite(audio_out[i]));
    }
  }

  /**
   * @brief Configures the filter based on the provided FilterState, computing resonance, drive, etc.
   *
   * @param filter_state The FilterState containing style, cutoff, resonance, gain, etc.
   */
  void DigitalSvf::setupFilter(const FilterState& filter_state) {
    // Basic parameter clamping and conversions
    midi_cutoff_ = filter_state.midi_cutoff;
    poly_float cutoff = utils::midiNoteToFrequency(filter_state.midi_cutoff);
    float min_nyquist = getSampleRate() * kMinNyquistMult;
    cutoff = utils::clamp(cutoff, kMinCutoff, min_nyquist);

    poly_float gain_decibels = utils::clamp(filter_state.gain, kMinGain, kMaxGain);
    poly_float gain_amplitude = utils::dbToMagnitude(gain_decibels);

    poly_float resonance_percent = utils::clamp(filter_state.resonance_percent, 0.0f, 1.0f);
    poly_float resonance_adjust = resonance_percent * resonance_percent * resonance_percent;
    poly_float resonance = utils::interpolate(min_resonance_, max_resonance_, resonance_adjust);
    if (drive_compensation_)
      drive_ = filter_state.drive / (resonance_adjust * 2.0f + 1.0f);
    else
      drive_ = filter_state.drive;

    post_multiply_ = gain_amplitude / utils::sqrt(filter_state.drive);
    resonance_ = poly_float(1.0f) / resonance;

    // Blend is typically in [-1..1], controlling low/band/high distribution
    poly_float blend = utils::clamp(filter_state.pass_blend - 1.0f, -1.0f, 1.0f);

    // Compute amounts for low_amount_, band_amount_, high_amount_ based on style
    // (some styles have specialized logic, e.g. dual notch band).
    if (filter_state.style == kDualNotchBand) {
      poly_float t = blend * 0.5f + 0.5f;
      poly_float drive_t = poly_float::min(-blend + 1.0f, 1.0f);
      poly_float drive_mult = -t + 2.0f;
      drive_ = utils::interpolate(filter_state.drive, drive_ * drive_mult, drive_t);

      low_amount_ = t;
      band_amount_ = 0.0f;
      high_amount_ = 1.0f;
    }
    else if (filter_state.style == kNotchPassSwap) {
      poly_float drive_t = poly_float::abs(blend);
      drive_ = utils::interpolate(filter_state.drive, drive_, drive_t);

      low_amount_ = utils::min(-blend + 1.0f, 1.0f);
      band_amount_ = 0.0f;
      high_amount_ = utils::min(blend + 1.0f, 1.0f);
    }
    else if (filter_state.style == kBandPeakNotch) {
      poly_float drive_t = poly_float::min(-blend + 1.0f, 1.0f);
      drive_ = utils::interpolate(filter_state.drive, drive_, drive_t);

      poly_float drive_inv_t = -drive_t + 1.0f;
      poly_float mult = utils::sqrt((drive_inv_t * drive_inv_t) * 0.5f + 0.5f);
      poly_float peak_band_value = -utils::max(-blend, 0.0f);
      low_amount_ = mult * (peak_band_value + 1.0f);
      band_amount_ = mult * (peak_band_value - blend + 1.0f) * 2.0f;
      high_amount_ = low_amount_;
    }
    else if (filter_state.style == kShelving) {
      drive_ = 1.0f;
      post_multiply_ = 1.0f;
      poly_float low_bell_t = utils::clamp(blend + 1.0f, 0.0f, 1.0f);
      poly_float bell_high_t = utils::clamp(blend, 0.0f, 1.0f);
      poly_float band_t = poly_float(1.0f) - blend * blend;

      poly_float amplitude_sqrt = utils::sqrt(gain_amplitude);
      poly_float amplitude_quartic = utils::sqrt(amplitude_sqrt);
      poly_float mult_adjust = futils::pow(amplitude_quartic, blend);

      low_amount_ = utils::interpolate(gain_amplitude, 1.0f, low_bell_t);
      high_amount_ = utils::interpolate(1.0f, gain_amplitude, bell_high_t);
      band_amount_ = resonance_ * amplitude_sqrt * utils::interpolate(1.0f, amplitude_sqrt, band_t);
      midi_cutoff_ += utils::ratioToMidiTranspose(mult_adjust);
    }
    else {
      band_amount_ = utils::sqrt(-blend * blend + 1.0f);
      poly_mask blend_mask = poly_float::lessThan(blend, 0.0f);
      low_amount_ = (-blend) & blend_mask;
      high_amount_ = blend & ~blend_mask;
    }

    blends1_.v0 = 0.0f;
    blends1_.v1 = band_amount_;
    blends1_.v2 = low_amount_;

    blends2_.v0 = 0.0f;
    blends2_.v1 = band_amount_;
    blends2_.v2 = high_amount_;

    blends1_.v0 += high_amount_;
    blends1_.v1 += -resonance_ * high_amount_;
    blends1_.v2 += -high_amount_;

    blends2_.v0 += low_amount_;
    blends2_.v1 += -resonance_ * low_amount_;
    blends2_.v2 += -low_amount_;
  }

  void DigitalSvf::setResonanceBounds(mono_float min, mono_float max) {
    min_resonance_ = min;
    max_resonance_ = max;
  }

  /**
   * @brief Tick function for advanced, saturating 12 dB filtering.
   *
   * @param audio_in  The current input sample.
   * @param coefficient The computed filter coefficient.
   * @param resonance The filter resonance.
   * @param drive     Input drive multiplier.
   * @param blends    The filter’s low/band/high mix.
   * @return The final processed sample with saturation.
   */
  force_inline poly_float DigitalSvf::tick(poly_float audio_in, poly_float coefficient,
                                           poly_float resonance, poly_float drive, FilterValues& blends) {
    return futils::hardTanh(tickBasic(audio_in, coefficient, resonance, drive, blends));
  }

  /**
   * @brief A simpler single tick for a 12 dB filter, skipping advanced distortion.
   *
   * @param audio_in   The input sample.
   * @param coefficient The filter coefficient from the lookup.
   * @param resonance  The filter resonance (inverted).
   * @param drive      The input drive multiplier.
   * @param blends     Low/band/high mix factors.
   * @return Filtered sample output.
   */
  force_inline poly_float DigitalSvf::tickBasic(poly_float audio_in, poly_float coefficient,
                                                poly_float resonance, poly_float drive, FilterValues& blends) {
    poly_float coefficient_squared = coefficient * coefficient;
    poly_float coefficient_0 = poly_float(1.0f) / (coefficient_squared + coefficient * resonance + 1.0f);
    poly_float coefficient_1 = coefficient_0 * coefficient;
    poly_float coefficient_2 = coefficient_0 * coefficient_squared;
    poly_float in = drive * audio_in;

    poly_float v3 = in - ic2eq_;
    poly_float v1 = utils::mulAdd(coefficient_0 * ic1eq_, coefficient_1, v3);
    poly_float v2 = utils::mulAdd(utils::mulAdd(ic2eq_, coefficient_1, ic1eq_), coefficient_2, v3);
    ic1eq_ = v1 * 2.0f - ic1eq_;
    ic2eq_ = v2 * 2.0f - ic2eq_;

    return utils::mulAdd(utils::mulAdd(blends.v0 * in, blends.v1, v1), blends.v2, v2);
  }

  /**
   * @brief Tick function for 24 dB filtering, performing a pre-stage, saturating, then a second SVF pass.
   *
   * @param audio_in   The input sample.
   * @param coefficient The filter coefficient.
   * @param resonance  The filter resonance.
   * @param drive      Input drive multiplier.
   * @param blends     Filter mixing parameters.
   * @return Filtered sample after 24 dB of attenuation.
   */
  force_inline poly_float DigitalSvf::tick24(poly_float audio_in, poly_float coefficient,
                                             poly_float resonance, poly_float drive, FilterValues& blends) {
    poly_float coefficient_squared = coefficient * coefficient;
    poly_float pre_coefficient_0 = poly_float(1.0f) / (coefficient_squared + coefficient + 1.0f);
    poly_float pre_coefficient_1 = pre_coefficient_0 * coefficient;
    poly_float pre_coefficient_2 = pre_coefficient_0 * coefficient_squared;

    poly_float in = drive * audio_in;

    poly_float v3_pre = in - ic2eq_pre_;
    poly_float v1_pre = utils::mulAdd(pre_coefficient_0 * ic1eq_pre_, pre_coefficient_1, v3_pre);
    poly_float v2_pre = utils::mulAdd(utils::mulAdd(ic2eq_pre_, pre_coefficient_1, ic1eq_pre_),
                                      pre_coefficient_2, v3_pre);
    ic1eq_pre_ = v1_pre * 2.0f - ic1eq_pre_;
    ic2eq_pre_ = v2_pre * 2.0f - ic2eq_pre_;
    poly_float out_pre = utils::mulAdd(utils::mulAdd(blends.v0 * in, blends.v1, v1_pre), blends.v2, v2_pre);

    poly_float distort = futils::hardTanh(out_pre);

    return tick(distort, coefficient, resonance, 1.0f, blends);
  }

  /**
   * @brief Basic, non-distorting 24 dB filter tick.
   *
   * @param audio_in   The input sample.
   * @param coefficient The filter coefficient.
   * @param resonance  The filter resonance.
   * @param drive      Input drive multiplier.
   * @param blends     Filter mixing parameters.
   * @return Final 24 dB filter output.
   */
  force_inline poly_float DigitalSvf::tickBasic24(poly_float audio_in, poly_float coefficient,
                                                  poly_float resonance, poly_float drive, FilterValues& blends) {
    poly_float coefficient_squared = coefficient * coefficient;
    poly_float pre_coefficient_0 = poly_float(1.0f) / (coefficient_squared + coefficient + 1.0f);
    poly_float pre_coefficient_1 = pre_coefficient_0 * coefficient;
    poly_float pre_coefficient_2 = pre_coefficient_0 * coefficient_squared;

    poly_float v3_pre = audio_in - ic2eq_pre_;
    poly_float v1_pre = utils::mulAdd(pre_coefficient_0 * ic1eq_pre_, pre_coefficient_1, v3_pre);
    poly_float v2_pre = utils::mulAdd(utils::mulAdd(ic2eq_pre_, pre_coefficient_1, ic1eq_pre_),
                                      pre_coefficient_2, v3_pre);
    ic1eq_pre_ = v1_pre * 2.0f - ic1eq_pre_;
    ic2eq_pre_ = v2_pre * 2.0f - ic2eq_pre_;
    poly_float out_pre = utils::mulAdd(utils::mulAdd(blends.v0 * audio_in, blends.v1, v1_pre), blends.v2, v2_pre);

    return tickBasic(out_pre, coefficient, resonance, drive, blends);
  }

  /**
   * @brief A dual-stage filter approach, e.g., for dual notch/band passes.
   *
   * @param audio_in   The input sample.
   * @param coefficient The filter coefficient.
   * @param resonance  Filter resonance.
   * @param drive      Input drive multiplier.
   * @param blends1    The first filter stage blends.
   * @param blends2    The second filter stage blends.
   * @return The final sample after going through both filter passes.
   */
  force_inline poly_float DigitalSvf::tickDual(poly_float audio_in, poly_float coefficient,
                                               poly_float resonance, poly_float drive,
                                               FilterValues& blends1, FilterValues& blends2) {
    poly_float coefficient_squared = coefficient * coefficient;
    poly_float pre_coefficient_0 = poly_float(1.0f) / (coefficient_squared + coefficient + 1.0f);
    poly_float pre_coefficient_1 = pre_coefficient_0 * coefficient;
    poly_float pre_coefficient_2 = pre_coefficient_0 * coefficient_squared;
    poly_float coefficient_0 = poly_float(1.0f) / (coefficient_squared + coefficient * resonance + 1.0f);
    poly_float coefficient_1 = coefficient_0 * coefficient;
    poly_float coefficient_2 = coefficient_0 * coefficient_squared;

    poly_float in = drive * audio_in;

    poly_float v3_pre = in - ic2eq_pre_;
    poly_float v1_pre = utils::mulAdd(pre_coefficient_0 * ic1eq_pre_, pre_coefficient_1, v3_pre);
    poly_float v2_pre = utils::mulAdd(utils::mulAdd(ic2eq_pre_, pre_coefficient_1, ic1eq_pre_),
                                      pre_coefficient_2, v3_pre);
    ic1eq_pre_ = v1_pre * 2.0f - ic1eq_pre_;
    ic2eq_pre_ = v2_pre * 2.0f - ic2eq_pre_;
    poly_float out_pre = utils::mulAdd(utils::mulAdd(blends1.v0 * in, blends1.v1, v1_pre), blends1.v2, v2_pre);

    poly_float distort = futils::hardTanh(out_pre);

    poly_float v3 = distort - ic2eq_;
    poly_float v1 = utils::mulAdd(coefficient_0 * ic1eq_, coefficient_1, v3);
    poly_float v2 = utils::mulAdd(utils::mulAdd(ic2eq_, coefficient_1, ic1eq_), coefficient_2, v3);
    ic1eq_ = v1 * 2.0f - ic1eq_;
    ic2eq_ = v2 * 2.0f - ic2eq_;

    return futils::hardTanh(utils::mulAdd(utils::mulAdd(blends2.v0 * distort, blends2.v1, v1), blends2.v2, v2));
  }

  /**
   * @brief Resets specified voices in the filter’s internal state variables.
   *
   * @param reset_masks The poly_mask selecting which voices to reset.
   */
  void DigitalSvf::reset(poly_mask reset_mask) {
    ic1eq_pre_ = utils::maskLoad(ic1eq_pre_, 0.0f, reset_mask);
    ic2eq_pre_ = utils::maskLoad(ic2eq_pre_, 0.0f, reset_mask);
    ic1eq_ = utils::maskLoad(ic1eq_, 0.0f, reset_mask);
    ic2eq_ = utils::maskLoad(ic2eq_, 0.0f, reset_mask);
  }

  /**
   * @brief Performs a complete reset of all internal states for every voice.
   */
  void DigitalSvf::hardReset() {
    reset(constants::kFullMask);
    resonance_ = 1.0f;
    blends1_.hardReset();
    blends2_.hardReset();

    low_amount_ = 0.0f;
    band_amount_ = 0.0f;
    high_amount_ = 0.0f;

    drive_ = 0.0f;
    post_multiply_ = 0.0f;
  }
} // namespace vital
