#include "delay.h"

#include "futils.h"
#include "memory.h"
#include "synth_constants.h"

namespace vital {

  namespace {
    /**
     * @brief Saturates the given sample via a hard-tanh function.
     *
     * @param value The sample value to saturate.
     * @return Saturated sample value.
     */
    force_inline poly_float saturate(poly_float value) {
      return futils::hardTanh(value);
    }

    /**
     * @brief Saturates the sample more aggressively, scaling down then up around a ratio.
     *
     * @param value The sample value to saturate.
     * @return Strongly saturated sample value.
     */
    force_inline poly_float saturateLarge(poly_float value) {
      static constexpr float kRatio = 8.0f;
      static constexpr float kMult = 1.0f / kRatio;
      return futils::hardTanh(value * kMult) * kRatio;
    }
  }

  //============================================================================
  //
  // Delay - Templated Methods
  //
  //============================================================================

  /**
   * @brief Resets the delay memory, low-pass, and high-pass filters to default states.
   */
  template<class MemoryType>
  void Delay<MemoryType>::hardReset() {
    memory_->clearAll();

    filter_gain_ = 0.0f;
    low_pass_.reset(constants::kFullMask);
    high_pass_.reset(constants::kFullMask);
  }

  /**
   * @brief Allocates a new memory buffer with @p max_samples and clamps the current delay period.
   *
   * @param max_samples Maximum samples for the new buffer size.
   */
  template<class MemoryType>
  void Delay<MemoryType>::setMaxSamples(int max_samples) {
    memory_ = std::make_unique<MemoryType>(max_samples);
    period_ = utils::min(period_, max_samples - 1);
  }

  /**
   * @brief Processes the delay by reading from the kAudio input buffer.
   *
   * @param num_samples Number of samples to process.
   */
  template<class MemoryType>
  void Delay<MemoryType>::process(int num_samples) {
    VITAL_ASSERT(inputMatchesBufferSize(kAudio));
    processWithInput(input(kAudio)->source->buffer, num_samples);
  }

  /**
   * @brief The main entry point for delay processing, selecting a style and applying transformations.
   *
   * Calculates smooth transitions of parameters (wet, dry, feedback, etc.), then calls one of
   * the style-specific process methods.
   *
   * @param audio_in    Pointer to the input buffer.
   * @param num_samples Number of samples to process.
   */
  template<class MemoryType>
  void Delay<MemoryType>::processWithInput(const poly_float* audio_in, int num_samples) {
    VITAL_ASSERT(checkInputAndOutputSize(num_samples));

    // Cache current values
    poly_float current_wet = wet_;
    poly_float current_dry = dry_;
    poly_float current_feedback = feedback_;
    poly_float current_period = period_;
    poly_float current_filter_gain = filter_gain_;
    poly_float current_low_coefficient = low_coefficient_;
    poly_float current_high_coefficient = high_coefficient_;

    // Retrieve target frequency
    poly_float target_frequency = input(kFrequency)->at(0);
    Style style = static_cast<Style>(static_cast<int>(input(kStyle)->at(0)[0]));

    // Use auxiliary frequency for stereo-based styles
    if (style == kStereo || style == kPingPong || style == kMidPingPong)
      target_frequency = utils::maskLoad(target_frequency, input(kFrequencyAux)->at(0), constants::kRightMask);

    // Smooth frequency changes
    poly_float decay = futils::exp_half(num_samples / (kDelayHalfLife * getSampleRate()));
    last_frequency_ = utils::interpolate(target_frequency, last_frequency_, decay);

    // Compute new wet/dry feedback
    poly_float wet = utils::clamp(input(kWet)->at(0), 0.0f, 1.0f);
    wet_ = futils::equalPowerFade(wet);
    dry_ = futils::equalPowerFadeInverse(wet);
    feedback_ = utils::clamp(input(kFeedback)->at(0), -1.0f, 1.0f);

    // Convert frequency to sample-based delay period
    poly_float samples = poly_float(getSampleRate()) / last_frequency_;

    // Adjust per-style (e.g. mid ping-pong offsets, forced feedback for ping-pong)
    if (style == kMidPingPong)
      samples += utils::swapStereo(samples) & constants::kLeftMask;
    if (style == kPingPong) {
      current_feedback = utils::maskLoad(current_feedback, 1.0f, constants::kRightMask);
      feedback_ = utils::maskLoad(feedback_, 1.0f, constants::kRightMask);
    }

    // Clamp to valid memory range
    period_ = utils::clamp(samples, 3.0f, memory_->getMaxPeriod());
    // Smooth period
    period_ = utils::interpolate(current_period, period_, 0.5f);

    // Setup filter coefficients
    poly_float filter_cutoff = input(kFilterCutoff)->at(0);
    poly_float filter_radius = getFilterRadius(input(kFilterSpread)->at(0));

    poly_float low_frequency = utils::midiNoteToFrequency(filter_cutoff + filter_radius);
    float min_nyquist = getSampleRate() * kMinNyquistMult;
    low_frequency = utils::clamp(low_frequency, 1.0f, min_nyquist);
    low_coefficient_ = OnePoleFilter<>::computeCoefficient(low_frequency, getSampleRate());

    poly_float high_frequency = utils::midiNoteToFrequency(filter_cutoff - filter_radius);
    high_frequency = utils::clamp(high_frequency, 1.0f, min_nyquist);
    high_coefficient_ = OnePoleFilter<>::computeCoefficient(high_frequency, getSampleRate());

    filter_gain_ = high_frequency / low_frequency + 1.0f;

    // Get damping frequency
    poly_float damping = utils::clamp(input(kDamping)->at(0), 0.0f, 1.0f);
    poly_float damping_note = utils::interpolate(kMinDampNote, kMaxDampNote, damping);
    poly_float damping_frequency = utils::midiNoteToFrequency(damping_note);

    // Choose processing style
    switch (style) {
      case kMono:
      case kStereo:
        process(audio_in, num_samples, current_period, current_feedback, current_filter_gain,
                current_low_coefficient, current_high_coefficient, current_wet, current_dry);
        break;
      case kPingPong:
        processMonoPingPong(audio_in, num_samples, current_period, current_feedback, current_filter_gain,
                            current_low_coefficient, current_high_coefficient, current_wet, current_dry);
        break;
      case kMidPingPong:
        processPingPong(audio_in, num_samples, current_period, current_feedback, current_filter_gain,
                        current_low_coefficient, current_high_coefficient, current_wet, current_dry);
        break;
      case kClampedDampened:
        damping_frequency = utils::clamp(damping_frequency, 1.0f, min_nyquist);
        low_coefficient_ = OnePoleFilter<>::computeCoefficient(damping_frequency, getSampleRate());
        processDamped(audio_in, num_samples, current_period, current_feedback,
                      current_low_coefficient, current_wet, current_dry);
        break;
      case kUnclampedUnfiltered:
        processCleanUnfiltered(audio_in, num_samples, current_period, current_feedback, current_wet, current_dry);
        break;
      default:
        processUnfiltered(audio_in, num_samples, current_period, current_feedback, current_wet, current_dry);
        break;
    }
  }

  /**
   * @brief Processes audio for a clean, unfiltered delay line (no saturation or filtering).
   */
  template<class MemoryType>
  void Delay<MemoryType>::processCleanUnfiltered(const poly_float* audio_in, int num_samples,
                                                 poly_float current_period, poly_float current_feedback,
                                                 poly_float current_wet, poly_float current_dry) {
    mono_float tick_increment = 1.0f / num_samples;
    poly_float delta_wet      = (wet_ - current_wet) * tick_increment;
    poly_float delta_dry      = (dry_ - current_dry) * tick_increment;
    poly_float delta_feedback = (feedback_ - current_feedback) * tick_increment;
    poly_float delta_period   = (period_ - current_period) * tick_increment;

    poly_float* dest = output()->buffer;

    for (int i = 0; i < num_samples; ++i) {
      current_feedback += delta_feedback;
      current_wet += delta_wet;
      current_dry += delta_dry;

      dest[i] = tickCleanUnfiltered(audio_in[i], current_period, current_feedback, current_wet, current_dry);
      current_period += delta_period;
    }
  }

  /**
   * @brief Processes audio with unfiltered memory but includes mild saturation in the feedback path.
   */
  template<class MemoryType>
  void Delay<MemoryType>::processUnfiltered(const poly_float* audio_in, int num_samples,
                                            poly_float current_period, poly_float current_feedback,
                                            poly_float current_wet, poly_float current_dry) {
    mono_float tick_increment = 1.0f / num_samples;
    poly_float delta_wet      = (wet_ - current_wet) * tick_increment;
    poly_float delta_dry      = (dry_ - current_dry) * tick_increment;
    poly_float delta_feedback = (feedback_ - current_feedback) * tick_increment;
    poly_float delta_period   = (period_ - current_period) * tick_increment;

    poly_float* dest = output()->buffer;

    for (int i = 0; i < num_samples; ++i) {
      current_feedback += delta_feedback;
      current_wet += delta_wet;
      current_dry += delta_dry;

      dest[i] = tickUnfiltered(audio_in[i], current_period, current_feedback, current_wet, current_dry);
      current_period += delta_period;
    }
  }

  /**
   * @brief Processes audio with filtering, saturation, and parameter smoothing.
   */
  template<class MemoryType>
  void Delay<MemoryType>::process(const poly_float* audio_in, int num_samples,
                                  poly_float current_period, poly_float current_feedback,
                                  poly_float current_filter_gain,
                                  poly_float current_low_coefficient, poly_float current_high_coefficient,
                                  poly_float current_wet, poly_float current_dry) {
    mono_float tick_increment = 1.0f / num_samples;
    poly_float delta_wet        = (wet_ - current_wet) * tick_increment;
    poly_float delta_dry        = (dry_ - current_dry) * tick_increment;
    poly_float delta_feedback   = (feedback_ - current_feedback) * tick_increment;
    poly_float delta_period     = (period_ - current_period) * tick_increment;
    poly_float delta_filter_gain = (filter_gain_ - current_filter_gain) * tick_increment;
    poly_float delta_low_coefficient = (low_coefficient_ - current_low_coefficient) * tick_increment;
    poly_float delta_high_coefficient = (high_coefficient_ - current_high_coefficient) * tick_increment;

    poly_float* dest = output()->buffer;

    for (int i = 0; i < num_samples; ++i) {
      current_feedback += delta_feedback;
      current_wet += delta_wet;
      current_dry += delta_dry;
      current_filter_gain += delta_filter_gain;
      current_low_coefficient += delta_low_coefficient;
      current_high_coefficient += delta_high_coefficient;

      dest[i] = tick(audio_in[i], current_period, current_feedback, current_filter_gain,
                     current_low_coefficient, current_high_coefficient, current_wet, current_dry);

      current_period += delta_period;
    }
  }

  /**
   * @brief Processes audio with a single low-pass filter for damping.
   */
  template<class MemoryType>
  void Delay<MemoryType>::processDamped(const poly_float* audio_in, int num_samples,
                                        poly_float current_period, poly_float current_feedback,
                                        poly_float current_low_coefficient,
                                        poly_float current_wet, poly_float current_dry) {
    mono_float tick_increment = 1.0f / num_samples;
    poly_float delta_wet        = (wet_ - current_wet) * tick_increment;
    poly_float delta_dry        = (dry_ - current_dry) * tick_increment;
    poly_float delta_feedback   = (feedback_ - current_feedback) * tick_increment;
    poly_float delta_period     = (period_ - current_period) * tick_increment;
    poly_float delta_low_coefficient = (low_coefficient_ - current_low_coefficient) * tick_increment;

    poly_float* dest = output()->buffer;

    for (int i = 0; i < num_samples; ++i) {
      current_feedback += delta_feedback;
      current_wet += delta_wet;
      current_dry += delta_dry;
      current_low_coefficient += delta_low_coefficient;

      dest[i] = tickDamped(audio_in[i], current_period, current_feedback,
                           current_low_coefficient, current_wet, current_dry);

      current_period += delta_period;
    }
  }

  /**
   * @brief Processes a stereo ping-pong delay, swapping channels and applying filters.
   */
  template<class MemoryType>
  void Delay<MemoryType>::processPingPong(const poly_float* audio_in, int num_samples,
                                          poly_float current_period, poly_float current_feedback,
                                          poly_float current_filter_gain,
                                          poly_float current_low_coefficient, poly_float current_high_coefficient,
                                          poly_float current_wet, poly_float current_dry) {
    mono_float tick_increment = 1.0f / num_samples;
    poly_float delta_wet        = (wet_ - current_wet) * tick_increment;
    poly_float delta_dry        = (dry_ - current_dry) * tick_increment;
    poly_float delta_feedback   = (feedback_ - current_feedback) * tick_increment;
    poly_float delta_period     = (period_ - current_period) * tick_increment;
    poly_float delta_filter_gain = (filter_gain_ - current_filter_gain) * tick_increment;
    poly_float delta_low_coefficient = (low_coefficient_ - current_low_coefficient) * tick_increment;
    poly_float delta_high_coefficient = (high_coefficient_ - current_high_coefficient) * tick_increment;

    poly_float* dest = output()->buffer;

    for (int i = 0; i < num_samples; ++i) {
      current_feedback += delta_feedback;
      current_wet += delta_wet;
      current_dry += delta_dry;
      current_filter_gain += delta_filter_gain;
      current_low_coefficient += delta_low_coefficient;
      current_high_coefficient += delta_high_coefficient;

      dest[i] = tickPingPong(audio_in[i], current_period, current_feedback, current_filter_gain,
                             current_low_coefficient, current_high_coefficient, current_wet, current_dry);

      current_period += delta_period;
    }
  }

  /**
   * @brief Processes a mono ping-pong delay, merging channels to mono before bouncing left/right.
   */
  template<class MemoryType>
  void Delay<MemoryType>::processMonoPingPong(const poly_float* audio_in, int num_samples,
                                              poly_float current_period, poly_float current_feedback,
                                              poly_float current_filter_gain,
                                              poly_float current_low_coefficient, poly_float current_high_coefficient,
                                              poly_float current_wet, poly_float current_dry) {
    mono_float tick_increment = 1.0f / num_samples;
    poly_float delta_wet        = (wet_ - current_wet) * tick_increment;
    poly_float delta_dry        = (dry_ - current_dry) * tick_increment;
    poly_float delta_feedback   = (feedback_ - current_feedback) * tick_increment;
    poly_float delta_period     = (period_ - current_period) * tick_increment;
    poly_float delta_filter_gain = (filter_gain_ - current_filter_gain) * tick_increment;
    poly_float delta_low_coefficient = (low_coefficient_ - current_low_coefficient) * tick_increment;
    poly_float delta_high_coefficient = (high_coefficient_ - current_high_coefficient) * tick_increment;

    poly_float* dest = output()->buffer;

    for (int i = 0; i < num_samples; ++i) {
      current_feedback += delta_feedback;
      current_wet += delta_wet;
      current_dry += delta_dry;
      current_filter_gain += delta_filter_gain;
      current_low_coefficient += delta_low_coefficient;
      current_high_coefficient += delta_high_coefficient;

      dest[i] = tickMonoPingPong(audio_in[i], current_period, current_feedback, current_filter_gain,
                                 current_low_coefficient, current_high_coefficient, current_wet, current_dry);

      current_period += delta_period;
    }
  }

  /**
   * @brief Single-sample tick for a clean, unfiltered delay.
   */
  template<class MemoryType>
  force_inline poly_float Delay<MemoryType>::tickCleanUnfiltered(poly_float audio_in, poly_float period,
                                                                 poly_float feedback,
                                                                 poly_float wet, poly_float dry) {
    poly_float read = memory_->get(period);
    memory_->push(audio_in + read * feedback);
    return dry * audio_in + wet * read;
  }

  /**
   * @brief Single-sample tick for an unfiltered delay including mild saturation.
   */
  template<class MemoryType>
  force_inline poly_float Delay<MemoryType>::tickUnfiltered(poly_float audio_in, poly_float period,
                                                            poly_float feedback,
                                                            poly_float wet, poly_float dry) {
    poly_float read = memory_->get(period);
    memory_->push(saturate(audio_in + read * feedback));
    return dry * audio_in + wet * read;
  }

  /**
   * @brief Single-sample tick for the main (filtered, saturated) delay process.
   */
  template<class MemoryType>
  force_inline poly_float Delay<MemoryType>::tick(poly_float audio_in, poly_float period, poly_float feedback,
                                                  poly_float filter_gain, poly_float low_coefficient,
                                                  poly_float high_coefficient,
                                                  poly_float wet, poly_float dry) {
    poly_float read = memory_->get(period);
    poly_float write_raw_value = saturateLarge(audio_in + read * feedback);
    poly_float low_pass_result = low_pass_.tickBasic(write_raw_value * filter_gain, low_coefficient);
    poly_float second_pass_result = high_pass_.tickBasic(low_pass_result, high_coefficient);
    memory_->push(low_pass_result - second_pass_result);
    return dry * audio_in + wet * read;
  }

  /**
   * @brief Single-sample tick for a damped delay (low-pass filtering only).
   */
  template<class MemoryType>
  force_inline poly_float Delay<MemoryType>::tickDamped(poly_float audio_in, poly_float period,
                                                        poly_float feedback, poly_float low_coefficient,
                                                        poly_float wet, poly_float dry) {
    poly_float read = memory_->get(period);
    poly_float write_raw_value = saturateLarge(audio_in + read * feedback);
    poly_float low_pass_result = low_pass_.tickBasic(write_raw_value, low_coefficient);
    memory_->push(low_pass_result);
    return dry * audio_in + wet * read;
  }

  /**
   * @brief Single-sample tick for a stereo ping-pong delay, swapping channels.
   */
  template<class MemoryType>
  force_inline poly_float Delay<MemoryType>::tickPingPong(poly_float audio_in, poly_float period, poly_float feedback,
                                                          poly_float filter_gain, poly_float low_coefficient,
                                                          poly_float high_coefficient,
                                                          poly_float wet, poly_float dry) {
    poly_float read = memory_->get(period);
    poly_float write_raw_value = utils::swapStereo(saturateLarge(audio_in + read * feedback));
    poly_float low_pass_result = low_pass_.tickBasic(write_raw_value * filter_gain, low_coefficient);
    poly_float second_pass_result = high_pass_.tickBasic(low_pass_result, high_coefficient);
    memory_->push(low_pass_result - second_pass_result);
    return dry * audio_in + wet * read;
  }

  /**
   * @brief Single-sample tick for a mono ping-pong delay, merging channels before swap.
   */
  template<class MemoryType>
  force_inline poly_float Delay<MemoryType>::tickMonoPingPong(poly_float audio_in, poly_float period,
                                                              poly_float feedback,
                                                              poly_float filter_gain, poly_float low_coefficient,
                                                              poly_float high_coefficient,
                                                              poly_float wet, poly_float dry) {
    poly_float read = memory_->get(period);
    poly_float mono_in = (audio_in + utils::swapStereo(audio_in)) * (1.0f / kSqrt2) & constants::kLeftMask;
    poly_float write_raw_value = utils::swapStereo(saturateLarge(mono_in + read * feedback));
    poly_float low_pass_result = low_pass_.tickBasic(write_raw_value * filter_gain, low_coefficient);
    poly_float second_pass_result = high_pass_.tickBasic(low_pass_result, high_coefficient);
    memory_->push(low_pass_result - second_pass_result);
    return dry * audio_in + wet * read;
  }

  // Explicit instantiations of Delay for StereoMemory and Memory types.
  template class Delay<StereoMemory>;
  template class Delay<Memory>;
} // namespace vital
