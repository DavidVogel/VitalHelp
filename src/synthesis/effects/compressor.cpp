#include "compressor.h"
#include "futils.h"

namespace vital {

  namespace {
    /**
     * @brief Default window size (in seconds) for RMS calculation.
     */
    constexpr mono_float kRmsTime = 0.025f;

    /**
     * @brief Maximum allowed ratio for expansion.
     */
    constexpr mono_float kMaxExpandMult = 32.0f;

    /**
     * @brief Base attack times for the three bands (low, band, high).
     */
    constexpr mono_float kLowAttackMs = 2.8f;
    constexpr mono_float kBandAttackMs = 1.4f;
    constexpr mono_float kHighAttackMs = 0.7f;

    /**
     * @brief Base release times for the three bands (low, band, high).
     */
    constexpr mono_float kLowReleaseMs = 40.0f;
    constexpr mono_float kBandReleaseMs = 28.0f;
    constexpr mono_float kHighReleaseMs = 15.0f;

    /**
     * @brief Range of valid output gain (in dB).
     */
    constexpr mono_float kMinGain = -30.0f;
    constexpr mono_float kMaxGain = 30.0f;

    /**
     * @brief Range of valid threshold values (in dB).
     */
    constexpr mono_float kMinThreshold = -100.0f;
    constexpr mono_float kMaxThreshold = 12.0f;

    /**
     * @brief Minimum envelope size in samples for the compressor’s attack/release calculations.
     */
    constexpr mono_float kMinSampleEnvelope = 5.0f;
  } // namespace

  //============================================================================
  //
  // Compressor
  //
  //============================================================================

  /**
   * @brief Constructs a Compressor with specified base attack and release times for two voices.
   *
   * Initializes Processor with the correct number of inputs/outputs and sets envelope states.
   */
  Compressor::Compressor(mono_float base_attack_ms_first,
                         mono_float base_release_ms_first,
                         mono_float base_attack_ms_second,
                         mono_float base_release_ms_second)
    : Processor(kNumInputs, kNumOutputs),
      input_mean_squared_(0.0f) {
    // Use maskLoad to selectively load attack/release for voice 1 or voice 2
    base_attack_ms_ = utils::maskLoad(poly_float(base_attack_ms_second),
                                      base_attack_ms_first,
                                      constants::kFirstMask);
    poly_float second_release = base_release_ms_second;
    base_release_ms_ = utils::maskLoad(second_release,
                                       base_release_ms_first,
                                       constants::kFirstMask);
    output_mult_ = 0.0f;
    mix_ = 0.0f;
  }

  /**
   * @brief Processes audio by pulling from kAudio input and writing to kAudioOut output.
   *
   * @param num_samples Number of samples to process.
   */
  void Compressor::process(int num_samples) {
    processWithInput(input(kAudio)->source->buffer, num_samples);
  }

  /**
   * @brief Processes audio with the given input buffer.
   *
   * Calls processRms() to calculate RMS levels and then adjusts output volume accordingly.
   *
   * @param audio_in    Pointer to the input buffer.
   * @param num_samples Number of samples to process.
   */
  void Compressor::processWithInput(const poly_float* audio_in, int num_samples) {
    processRms(audio_in, num_samples);

    // Update RMS for input/output signals
    input_mean_squared_ = computeMeanSquared(audio_in, num_samples, input_mean_squared_);
    output_mean_squared_ = computeMeanSquared(output(kAudioOut)->buffer, num_samples, output_mean_squared_);
    scaleOutput(audio_in, num_samples);
  }

  /**
   * @brief Computes a running envelope (RMS) for upper/lower thresholds and applies compression gain.
   *
   * @param audio_in    Pointer to the input buffer.
   * @param num_samples Number of samples to process.
   */
  void Compressor::processRms(const poly_float* audio_in, int num_samples) {
    poly_float* audio_out = output(kAudioOut)->buffer;

    // Convert ms to samples
    mono_float samples_per_ms = (1.0f * getSampleRate()) / kMsPerSec;
    poly_float attack_mult = base_attack_ms_ * samples_per_ms;
    poly_float release_mult = base_release_ms_ * samples_per_ms;

    // Convert 0..1 from GUI to an exponential in the range [-4, 4], then map to sample-based envelope
    poly_float attack_exponent  = utils::clamp(input(kAttack)->at(0), 0.0f, 1.0f) * 8.0f - 4.0f;
    poly_float release_exponent = utils::clamp(input(kRelease)->at(0), 0.0f, 1.0f) * 8.0f - 4.0f;
    poly_float envelope_attack_samples  = futils::exp(attack_exponent) * attack_mult;
    poly_float envelope_release_samples = futils::exp(release_exponent) * release_mult;

    // Ensure a minimum envelope length
    envelope_attack_samples  = utils::max(envelope_attack_samples, kMinSampleEnvelope);
    envelope_release_samples = utils::max(envelope_release_samples, kMinSampleEnvelope);

    // Compute scales for attack/release
    poly_float attack_scale  = poly_float(1.0f) / (envelope_attack_samples + 1.0f);
    poly_float release_scale = poly_float(1.0f) / (envelope_release_samples + 1.0f);

    // Load and convert thresholds from dB to linear magnitude squared
    poly_float upper_threshold = utils::clamp(input(kUpperThreshold)->at(0), kMinThreshold, kMaxThreshold);
    upper_threshold = futils::dbToMagnitude(upper_threshold);
    upper_threshold *= upper_threshold;

    poly_float lower_threshold = utils::clamp(input(kLowerThreshold)->at(0), kMinThreshold, kMaxThreshold);
    lower_threshold = futils::dbToMagnitude(lower_threshold);
    lower_threshold *= lower_threshold;

    // Load compression/expansion ratios
    poly_float upper_ratio = utils::clamp(input(kUpperRatio)->at(0), 0.0f, 1.0f) * 0.5f;
    poly_float lower_ratio = utils::clamp(input(kLowerRatio)->at(0), -1.0f, 1.0f) * 0.5f;

    poly_float low_enveloped_mean_squared = low_enveloped_mean_squared_;
    poly_float high_enveloped_mean_squared = high_enveloped_mean_squared_;

    for (int i = 0; i < num_samples; ++i) {
      poly_float sample = audio_in[i];
      poly_float sample_squared = sample * sample;

      // Update high band envelope
      poly_mask high_attack_mask = poly_float::greaterThan(sample_squared, high_enveloped_mean_squared);
      poly_float high_samples = utils::maskLoad(envelope_release_samples, envelope_attack_samples, high_attack_mask);
      poly_float high_scale = utils::maskLoad(release_scale, attack_scale, high_attack_mask);
      high_enveloped_mean_squared = (sample_squared + high_enveloped_mean_squared * high_samples) * high_scale;
      high_enveloped_mean_squared = utils::max(high_enveloped_mean_squared, upper_threshold);

      // Compute compression above upper threshold
      poly_float upper_mag_delta = upper_threshold / high_enveloped_mean_squared;
      poly_float upper_mult = futils::pow(upper_mag_delta, upper_ratio);

      // Update low band envelope
      poly_mask low_attack_mask = poly_float::greaterThan(sample_squared, low_enveloped_mean_squared);
      poly_float low_samples = utils::maskLoad(envelope_release_samples, envelope_attack_samples, low_attack_mask);
      poly_float low_scale = utils::maskLoad(release_scale, attack_scale, low_attack_mask);
      low_enveloped_mean_squared = (sample_squared + low_enveloped_mean_squared * low_samples) * low_scale;
      low_enveloped_mean_squared = utils::min(low_enveloped_mean_squared, lower_threshold);

      // Compute expansion below lower threshold
      poly_float lower_mag_delta = lower_threshold / low_enveloped_mean_squared;
      poly_float lower_mult = futils::pow(lower_mag_delta, lower_ratio);

      // Combine gain multipliers and clamp
      poly_float gain_compression = utils::clamp(upper_mult * lower_mult, 0.0f, kMaxExpandMult);
      audio_out[i] = gain_compression * sample;

      VITAL_ASSERT(utils::isContained(audio_out[i]));
    }

    // Save updated envelopes
    low_enveloped_mean_squared_ = low_enveloped_mean_squared;
    high_enveloped_mean_squared_ = high_enveloped_mean_squared;
  }

  /**
   * @brief Applies output gain and dry/wet mix to the processed audio signal.
   *
   * @param audio_input Pointer to the original unprocessed input (for the dry signal).
   * @param num_samples Number of samples to process.
   */
  void Compressor::scaleOutput(const poly_float* audio_input, int num_samples) {
    poly_float* audio_out = output(kAudioOut)->buffer;

    // Interpolate output gain changes
    poly_float current_output_mult = output_mult_;
    poly_float gain = utils::clamp(input(kOutputGain)->at(0), kMinGain, kMaxGain);
    output_mult_ = futils::dbToMagnitude(gain);
    poly_float delta_output_mult = (output_mult_ - current_output_mult) * (1.0f / num_samples);

    // Interpolate mix changes
    poly_float current_mix = mix_;
    mix_ = utils::clamp(input(kMix)->at(0), 0.0f, 1.0f);
    poly_float delta_mix = (mix_ - current_mix) * (1.0f / num_samples);

    for (int i = 0; i < num_samples; ++i) {
      current_output_mult += delta_output_mult;
      current_mix += delta_mix;

      // Blend dry (audio_input) and wet (compressed audio_out * current_output_mult)
      audio_out[i] = utils::interpolate(audio_input[i], audio_out[i] * current_output_mult, current_mix);

      VITAL_ASSERT(utils::isContained(audio_out[i]));
    }
  }

  /**
   * @brief Resets the compressor’s internal RMS, envelopes, and gains.
   *
   * @param reset_mask A poly_mask specifying which voices should be reset.
   */
  void Compressor::reset(poly_mask reset_mask) {
    input_mean_squared_ = 0.0f;
    output_mean_squared_ = 0.0f;
    output_mult_ = 0.0f;
    mix_ = 0.0f;
    high_enveloped_mean_squared_ = 0.0f;
    low_enveloped_mean_squared_ = 0.0f;
  }

  /**
   * @brief Calculates a rolling mean squared value (RMS) over the input buffer.
   *
   * @param audio_in    Pointer to the audio data.
   * @param num_samples Number of samples to process.
   * @param mean_squared The previous mean squared value to be updated.
   * @return Updated mean squared value after processing all samples.
   */
  poly_float Compressor::computeMeanSquared(const poly_float* audio_in,
                                            int num_samples,
                                            poly_float mean_squared) {
    int rms_samples = kRmsTime * getSampleRate();
    float rms_adjusted = rms_samples - 1.0f;
    mono_float input_scale = 1.0f / rms_samples;

    for (int i = 0; i < num_samples; ++i) {
      poly_float sample = audio_in[i];
      poly_float sample_squared = sample * sample;
      mean_squared = (mean_squared * rms_adjusted + sample_squared) * input_scale;
    }
    return mean_squared;
  }

  //============================================================================
  //
  // MultibandCompressor
  //
  //============================================================================

  /**
   * @brief Constructs a MultibandCompressor with specialized LinkwitzRiley filters
   *        and separate Compressor objects for low and band/high processing.
   */
  MultibandCompressor::MultibandCompressor()
    : Processor(kNumInputs, kNumOutputs),
      low_band_filter_(120.0f),
      band_high_filter_(2500.0f),
      low_band_compressor_(kLowAttackMs, kLowReleaseMs, kBandAttackMs, kBandReleaseMs),
      band_high_compressor_(kBandAttackMs, kBandReleaseMs, kHighAttackMs, kHighReleaseMs) {
    was_low_enabled_ = false;
    was_high_enabled_ = false;

    // Connect band thresholds/ratios to the low band compressor
    low_band_compressor_.plug(&low_band_upper_threshold_, Compressor::kUpperThreshold);
    low_band_compressor_.plug(&low_band_lower_threshold_, Compressor::kLowerThreshold);
    low_band_compressor_.plug(&low_band_upper_ratio_, Compressor::kUpperRatio);
    low_band_compressor_.plug(&low_band_lower_ratio_, Compressor::kLowerRatio);
    low_band_compressor_.plug(&low_band_output_gain_, Compressor::kOutputGain);
    low_band_compressor_.useInput(input(kAttack), Compressor::kAttack);
    low_band_compressor_.useInput(input(kRelease), Compressor::kRelease);
    low_band_compressor_.useInput(input(kMix), Compressor::kMix);

    // Connect band thresholds/ratios to the band+high compressor
    band_high_compressor_.plug(&band_high_upper_threshold_, Compressor::kUpperThreshold);
    band_high_compressor_.plug(&band_high_lower_threshold_, Compressor::kLowerThreshold);
    band_high_compressor_.plug(&band_high_upper_ratio_, Compressor::kUpperRatio);
    band_high_compressor_.plug(&band_high_lower_ratio_, Compressor::kLowerRatio);
    band_high_compressor_.plug(&band_high_output_gain_, Compressor::kOutputGain);
    band_high_compressor_.useInput(input(kAttack), Compressor::kAttack);
    band_high_compressor_.useInput(input(kRelease), Compressor::kRelease);
    band_high_compressor_.useInput(input(kMix), Compressor::kMix);
  }

  /**
   * @brief Sets the oversampling factor for the multiband compressor filters and internal compressors.
   *
   * @param oversample The oversampling factor.
   */
  void MultibandCompressor::setOversampleAmount(int oversample) {
    Processor::setOversampleAmount(oversample);
    low_band_filter_.setOversampleAmount(oversample);
    band_high_filter_.setOversampleAmount(oversample);
    low_band_compressor_.setOversampleAmount(oversample);
    band_high_compressor_.setOversampleAmount(oversample);
  }

  /**
   * @brief Configures the sample rate for internal filters and compressors.
   *
   * @param sample_rate The new sample rate in Hz.
   */
  void MultibandCompressor::setSampleRate(int sample_rate) {
    Processor::setSampleRate(sample_rate);
    low_band_filter_.setSampleRate(sample_rate);
    band_high_filter_.setSampleRate(sample_rate);
    low_band_compressor_.setSampleRate(sample_rate);
    band_high_compressor_.setSampleRate(sample_rate);
  }

  /**
   * @brief Resets filters and internal compressors, and clears mean squared outputs.
   *
   * @param reset_mask A poly_mask specifying which voices should be reset.
   */
  void MultibandCompressor::reset(poly_mask reset_mask) {
    low_band_filter_.reset(reset_mask);
    band_high_filter_.reset(reset_mask);
    low_band_compressor_.reset(reset_mask);
    band_high_compressor_.reset(reset_mask);

    // Reset output buffers for RMS displays
    output(kLowInputMeanSquared)->buffer[0] = 0.0f;
    output(kLowOutputMeanSquared)->buffer[0] = 0.0f;
    output(kBandInputMeanSquared)->buffer[0] = 0.0f;
    output(kBandOutputMeanSquared)->buffer[0] = 0.0f;
    output(kHighInputMeanSquared)->buffer[0] = 0.0f;
    output(kHighOutputMeanSquared)->buffer[0] = 0.0f;
  }

  /**
   * @brief Processes audio by pulling from kAudio input and splitting/compressing bands as necessary.
   *
   * @param num_samples Number of samples to process.
   */
  void MultibandCompressor::process(int num_samples) {
    processWithInput(input(kAudio)->source->buffer, num_samples);
  }

  /**
   * @brief Packs the filter output into a single buffer for further band processing.
   *
   * This method merges low and high filter outputs in a way that suits Vital’s polyphonic approach.
   *
   * @param filter      Pointer to the LinkwitzRileyFilter.
   * @param num_samples Number of samples to process.
   * @param dest        Destination buffer for combined low/high results.
   */
  void MultibandCompressor::packFilterOutput(LinkwitzRileyFilter* filter, int num_samples, poly_float* dest) {
    const poly_float* low_output = filter->output(LinkwitzRileyFilter::kAudioLow)->buffer;
    const poly_float* high_output = filter->output(LinkwitzRileyFilter::kAudioHigh)->buffer;

    for (int i = 0; i < num_samples; ++i) {
      poly_float low_sample = low_output[i];
      poly_float high_sample = utils::swapVoices(high_output[i]);
      dest[i] = utils::maskLoad(high_sample, low_sample, constants::kFirstMask);
    }
  }

  /**
   * @brief Combines the low portion and band portion of the filter output for the low band compressor.
   *
   * @param num_samples Number of samples to process.
   * @param dest        Buffer into which the combined result is written.
   */
  void MultibandCompressor::packLowBandCompressor(int num_samples, poly_float* dest) {
    const poly_float* low_output = band_high_filter_.output(LinkwitzRileyFilter::kAudioLow)->buffer;
    const poly_float* high_output = band_high_filter_.output(LinkwitzRileyFilter::kAudioHigh)->buffer;

    for (int i = 0; i < num_samples; ++i) {
      poly_float low_band_sample = low_output[i];
      poly_float low_high_sample = high_output[i] & constants::kFirstMask;
      dest[i] = low_band_sample + low_high_sample;
    }
  }

  /**
   * @brief Writes outputs from both the low band and band/high compressors into one buffer.
   *
   * Used when both low and high bands are enabled (i.e., full multiband mode).
   *
   * @param num_samples Number of samples to process.
   * @param dest        Buffer for the combined output.
   */
  void MultibandCompressor::writeAllCompressorOutputs(int num_samples, poly_float* dest) {
    const poly_float* low_band_output = low_band_compressor_.output(Compressor::kAudioOut)->buffer;
    const poly_float* high_output = band_high_compressor_.output(Compressor::kAudioOut)->buffer;

    for (int i = 0; i < num_samples; ++i) {
      // Sum left/right voices for low band and high band separately
      poly_float low_band_sample = low_band_output[i];
      low_band_sample += utils::swapVoices(low_band_sample);
      poly_float high_sample = utils::swapVoices(high_output[i]);
      dest[i] = low_band_sample + high_sample;
    }
  }

  /**
   * @brief Writes the output of a single Compressor instance to the destination buffer,
   *        handling Vital’s polyphonic approach by swapping voices as needed.
   *
   * @param compressor  Pointer to the compressor whose output is being written.
   * @param num_samples Number of samples to process.
   * @param dest        Buffer for the output data.
   */
  void MultibandCompressor::writeCompressorOutputs(Compressor* compressor, int num_samples, poly_float* dest) {
    const poly_float* compressor_output = compressor->output(Compressor::kAudioOut)->buffer;

    for (int i = 0; i < num_samples; ++i) {
      poly_float sample = compressor_output[i];
      dest[i] = sample + utils::swapVoices(sample);
    }
  }

  /**
   * @brief Processes audio with a given input buffer, handling multiband routing based on enabled bands.
   *
   * Splits the audio into low, band, and high frequencies with Linkwitz-Riley filters,
   * feeds them to separate compressors, and combines or bypasses them depending on user settings.
   *
   * @param audio_in    Pointer to the input buffer.
   * @param num_samples Number of samples to process.
   */
  void MultibandCompressor::processWithInput(const poly_float* audio_in, int num_samples) {
    int enabled_bands = input(kEnabledBands)->at(0)[0];
    bool low_enabled = (enabled_bands == kMultiband || enabled_bands == kLowBand);
    bool high_enabled = (enabled_bands == kMultiband || enabled_bands == kHighBand);

    // Retrieve thresholds and ratios from the user inputs, applying them to low or band+high as needed
    low_band_upper_threshold_.buffer[0] = utils::maskLoad(input(kBandUpperThreshold)->at(0),
                                                          input(kLowUpperThreshold)->at(0),
                                                          constants::kFirstMask);
    band_high_upper_threshold_.buffer[0] = utils::maskLoad(input(kHighUpperThreshold)->at(0),
                                                           input(kBandUpperThreshold)->at(0),
                                                           constants::kFirstMask);
    low_band_lower_threshold_.buffer[0] = utils::maskLoad(input(kBandLowerThreshold)->at(0),
                                                          input(kLowLowerThreshold)->at(0),
                                                          constants::kFirstMask);
    band_high_lower_threshold_.buffer[0] = utils::maskLoad(input(kHighLowerThreshold)->at(0),
                                                           input(kBandLowerThreshold)->at(0),
                                                           constants::kFirstMask);
    low_band_upper_ratio_.buffer[0] = utils::maskLoad(input(kBandUpperRatio)->at(0),
                                                      input(kLowUpperRatio)->at(0),
                                                      constants::kFirstMask);
    band_high_upper_ratio_.buffer[0] = utils::maskLoad(input(kHighUpperRatio)->at(0),
                                                       input(kBandUpperRatio)->at(0),
                                                       constants::kFirstMask);
    low_band_lower_ratio_.buffer[0] = utils::maskLoad(input(kBandLowerRatio)->at(0),
                                                      input(kLowLowerRatio)->at(0),
                                                      constants::kFirstMask);
    band_high_lower_ratio_.buffer[0] = utils::maskLoad(input(kHighLowerRatio)->at(0),
                                                       input(kBandLowerRatio)->at(0),
                                                       constants::kFirstMask);

    low_band_output_gain_.buffer[0] = utils::maskLoad(input(kBandOutputGain)->at(0),
                                                      input(kLowOutputGain)->at(0),
                                                      constants::kFirstMask);
    band_high_output_gain_.buffer[0] = utils::maskLoad(input(kHighOutputGain)->at(0),
                                                       input(kBandOutputGain)->at(0),
                                                       constants::kFirstMask);

    // Reset filters/compressors if enabled band configuration changed
    if (low_enabled != was_low_enabled_ || high_enabled != was_high_enabled_) {
      low_band_filter_.reset(constants::kFullMask);
      band_high_filter_.reset(constants::kFullMask);
      low_band_compressor_.reset(constants::kFullMask);
      band_high_compressor_.reset(constants::kFullMask);
      was_low_enabled_ = low_enabled;
      was_high_enabled_ = high_enabled;
    }

    poly_float* audio_out = output(kAudioOut)->buffer;

    // Full multiband mode: low + band + high
    if (low_enabled && high_enabled) {
      // Split signal into low and band+high
      low_band_filter_.processWithInput(audio_in, num_samples);
      packFilterOutput(&low_band_filter_, num_samples, audio_out);

      // Split band+high further
      band_high_filter_.processWithInput(audio_out, num_samples);
      packLowBandCompressor(num_samples, audio_out);

      // Compress low band
      low_band_compressor_.processWithInput(audio_out, num_samples);

      // Compress high band
      const poly_float* band_high_buffer = band_high_filter_.output(LinkwitzRileyFilter::kAudioHigh)->buffer;
      band_high_compressor_.processWithInput(band_high_buffer, num_samples);

      // Combine all
      writeAllCompressorOutputs(num_samples, audio_out);
    }
    // Only low band
    else if (low_enabled) {
      low_band_filter_.processWithInput(audio_in, num_samples);
      packFilterOutput(&low_band_filter_, num_samples, audio_out);
      low_band_compressor_.processWithInput(audio_out, num_samples);
      writeCompressorOutputs(&low_band_compressor_, num_samples, audio_out);
    }
    // Only high band
    else if (high_enabled) {
      band_high_filter_.processWithInput(audio_in, num_samples);
      packFilterOutput(&band_high_filter_, num_samples, audio_out);
      band_high_compressor_.processWithInput(audio_out, num_samples);
      writeCompressorOutputs(&band_high_compressor_, num_samples, audio_out);
    }
    // Single band, no filter
    else {
      band_high_compressor_.processWithInput(audio_in, num_samples);
      utils::copyBuffer(audio_out, band_high_compressor_.output(Compressor::kAudioOut)->buffer, num_samples);
    }

    // Write mean squared values for GUI metering
    poly_float low_band_input_ms = low_band_compressor_.getInputMeanSquared();
    poly_float band_high_input_ms = band_high_compressor_.getInputMeanSquared();
    poly_float low_band_output_ms = low_band_compressor_.getOutputMeanSquared();
    poly_float band_high_output_ms = band_high_compressor_.getOutputMeanSquared();

    output(kLowInputMeanSquared)->buffer[0] = low_band_input_ms;
    output(kLowOutputMeanSquared)->buffer[0] = low_band_output_ms;

    if (low_enabled) {
      output(kBandInputMeanSquared)->buffer[0] = utils::swapVoices(low_band_input_ms);
      output(kBandOutputMeanSquared)->buffer[0] = utils::swapVoices(low_band_output_ms);
    }
    else {
      output(kBandInputMeanSquared)->buffer[0] = band_high_input_ms;
      output(kBandOutputMeanSquared)->buffer[0] = band_high_output_ms;
    }

    output(kHighInputMeanSquared)->buffer[0] = utils::swapVoices(band_high_input_ms);
    output(kHighOutputMeanSquared)->buffer[0] = utils::swapVoices(band_high_output_ms);
  }

} // namespace vital
