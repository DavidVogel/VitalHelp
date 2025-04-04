#include "phaser.h"

#include "operators.h"
#include "phaser_filter.h"

#include <climits>

namespace vital {

  /**
   * @brief Constructs a Phaser object and adds an internal @c PhaserFilter.
   *
   * Initializes default values for mix, modulation depth, phase offset,
   * and sets up the filter as an idle processor.
   */
  Phaser::Phaser()
    : ProcessorRouter(kNumInputs, kNumOutputs),
      mix_(0.0f),
      mod_depth_(0.0f),
      phase_offset_(0.0f),
      phase_(0) {
    phaser_filter_ = new PhaserFilter(/* oversample= */ true);
    addIdleProcessor(phaser_filter_);
  }

  /**
   * @brief Initializes the Phaser, connecting input parameters to the @c PhaserFilter.
   */
  void Phaser::init() {
    // Connect feedback gain and blend from the Phaser inputs to the PhaserFilter
    phaser_filter_->useInput(input(kFeedbackGain), PhaserFilter::kResonance);
    phaser_filter_->useInput(input(kBlend), PhaserFilter::kPassBlend);
    // Connect the cutoff output to the filter's MIDI cutoff input
    phaser_filter_->plug(&cutoff_, PhaserFilter::kMidiCutoff);

    phaser_filter_->init();
    ProcessorRouter::init();
  }

  /**
   * @brief Resets the Phaser, including the internal filter and cached parameters.
   */
  void Phaser::hardReset() {
    phaser_filter_->reset(constants::kFullMask);
    mod_depth_ = input(kModDepth)->at(0);
    phase_offset_ = input(kPhaseOffset)->at(0);
  }

  /**
   * @brief Processes a block of audio using the phaser effect by reading from the kAudio input.
   *
   * @param num_samples Number of samples to process.
   */
  void Phaser::process(int num_samples) {
    processWithInput(input(kAudio)->source->buffer, num_samples);
  }

  /**
   * @brief Processes audio with a given buffer, applying LFO modulation and phaser filtering.
   *
   * This method updates the LFO phase, applies modulation to the cutoff, and blends
   * the filter output with the original signal according to the mix parameter.
   *
   * @param audio_in    Pointer to the input audio buffer.
   * @param num_samples Number of samples to process.
   */
  void Phaser::processWithInput(const poly_float* audio_in, int num_samples) {
    VITAL_ASSERT(checkInputAndOutputSize(num_samples));

    // Compute how much the phase increments for each sample
    poly_float tick_delta = input(kRate)->at(0) * (1.0f / getSampleRate());
    poly_int tick_delta_phase = utils::toInt(tick_delta * UINT_MAX);

    // For gradually changing stereo phase offset
    mono_float tick_inc = 1.0f / num_samples;
    poly_float phase_spread = phase_offset_ * constants::kStereoSplit;
    poly_int phase_offset = utils::toInt(phase_spread * INT_MAX);
    phase_offset_ = input(kPhaseOffset)->at(0);
    poly_float end_spread = phase_offset_ * constants::kStereoSplit;
    poly_float delta_spread = (end_spread - phase_spread) * tick_inc;
    poly_int delta_phase_offset = utils::toInt(delta_spread * INT_MAX);

    // Smoothly transition the mod depth
    poly_float current_mod_depth = mod_depth_;
    mod_depth_ = input(kModDepth)->at(0);
    poly_float delta_depth = (mod_depth_ - current_mod_depth) * tick_inc;

    // Generate the cutoff values for each sample
    const poly_float* center_buffer = input(kCenter)->source->buffer;
    poly_int current_phase = phase_;
    for (int i = 0; i < num_samples; ++i) {
      phase_offset += delta_phase_offset;
      current_mod_depth += delta_depth;

      // Phase mod with potential folding
      poly_int shifted_phase = current_phase + phase_offset;
      poly_mask fold_mask = poly_int::greaterThan(shifted_phase, INT_MAX);
      poly_int folded_phase = utils::maskLoad(shifted_phase, -shifted_phase, fold_mask);

      // Convert folded_phase to a [-1..1] range
      poly_float modulation = utils::toFloat(folded_phase) * (2.0f / INT_MAX) - 1.0f;

      // Write cutoff in MIDI note space
      cutoff_.buffer[i] = center_buffer[i] + modulation * current_mod_depth;
    }

    // Process the audio through the phaser filter
    phaser_filter_->processWithInput(audio_in, num_samples);

    // Update the LFO phase
    phase_ += utils::toInt((tick_delta * num_samples) * UINT_MAX);

    // Blend the phaser output with the dry input
    poly_float current_mix = mix_;
    mix_ = utils::clamp(input(kMix)->at(0), 0.0f, 1.0f);
    poly_float delta_mix = (mix_ - current_mix) * (1.0f / num_samples);

    const poly_float* phaser_out = phaser_filter_->output()->buffer;
    poly_float* audio_out = output(kAudioOutput)->buffer;
    for (int i = 0; i < num_samples; ++i) {
      current_mix += delta_mix;
      audio_out[i] = utils::interpolate(audio_in[i], phaser_out[i], current_mix);
    }

    // Store the final cutoff value for UI or further processing
    output(kCutoffOutput)->buffer[0] = cutoff_.buffer[num_samples - 1];
  }

  /**
   * @brief Syncs the phaser's LFO phase to a given absolute time.
   *
   * @param seconds The playback time in seconds to synchronize to.
   */
  void Phaser::correctToTime(double seconds) {
    poly_float rate = input(kRate)->at(0);
    poly_float offset = utils::getCycleOffsetFromSeconds(seconds, rate);
    phase_ = utils::toInt((offset - 0.5f) * UINT_MAX) + INT_MAX / 2;
  }
} // namespace vital
