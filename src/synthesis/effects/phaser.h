#pragma once

#include "processor_router.h"
#include "phaser_filter.h"
#include "operators.h"

namespace vital {

  /**
   * @class Phaser
   * @brief A multi-stage phaser effect that modulates filter cutoff with a low-frequency oscillator.
   *
   * The Phaser class processes audio by routing it through a @c PhaserFilter and modulating
   * the filter's cutoff. It provides adjustable parameters like mix, rate, feedback, center,
   * modulation depth, and phase offset.
   */
  class PhaserFilter;

  class Phaser : public ProcessorRouter {
    public:
      /**
       * @enum InputIndices
       * @brief Enumerates the input indices for the Phaser.
       */
      enum {
        kAudio,        ///< Audio input buffer
        kMix,          ///< Dry/wet mix control
        kRate,         ///< LFO rate for cutoff modulation
        kFeedbackGain, ///< Amount of feedback in the phaser filter
        kCenter,       ///< Center frequency (MIDI note) for the phaser
        kModDepth,     ///< Modulation depth (amount of sweep)
        kPhaseOffset,  ///< LFO phase offset for stereo spread
        kBlend,        ///< Amount of pass/comb blend in the phaser
        kNumInputs     ///< Total number of inputs
      };

      /**
       * @enum OutputIndices
       * @brief Enumerates the output indices for the Phaser.
       */
      enum {
        kAudioOutput,  ///< Phaser audio output
        kCutoffOutput, ///< Current cutoff (MIDI note) at the final sample
        kNumOutputs    ///< Total number of outputs
      };

      /**
       * @brief Constructs a Phaser ProcessorRouter with default settings.
       */
      Phaser();

      /** @brief Virtual destructor. */
      virtual ~Phaser() { }

      /**
       * @brief Creates a clone of this Processor. (Not implemented for Phaser).
       * @return Returns nullptr.
       */
      virtual Processor* clone() const override { VITAL_ASSERT(false); return nullptr; }

      /**
       * @brief Processes a block of audio by pulling from the audio input buffer.
       *
       * @param num_samples Number of samples to process.
       */
      void process(int num_samples) override;

      /**
       * @brief Processes a block of audio using the provided input buffer.
       *
       * @param audio_in    Pointer to the audio input buffer.
       * @param num_samples Number of samples to process.
       */
      void processWithInput(const poly_float* audio_in, int num_samples) override;

      /**
       * @brief Initializes the phaser, hooking up internal connections.
       */
      void init() override;

      /**
       * @brief Resets internal states, filters, and stored parameters.
       */
      void hardReset() override;

      /**
       * @brief Corrects LFO phase according to an absolute time offset.
       *
       * Used to synchronize the phaser LFO to a specific playback time.
       *
       * @param seconds The playback time in seconds.
       */
      void correctToTime(double seconds);

      /**
       * @brief Sets oversampling for the phaser and allocates extra buffer size for internal use.
       *
       * @param oversample Oversampling factor (1x, 2x, etc.).
       */
      void setOversampleAmount(int oversample) override {
        ProcessorRouter::setOversampleAmount(oversample);
        cutoff_.ensureBufferSize(oversample * kMaxBufferSize);
      }

    private:
      /**
       * @brief An Output object storing the cutoff (in MIDI notes) for the phaser filter.
       */
      Output cutoff_;

      /**
       * @brief Pointer to the internal phaser filter Processor.
       */
      PhaserFilter* phaser_filter_;

      /**
       * @brief Current dry/wet mix (0 = fully dry, 1 = fully wet).
       */
      poly_float mix_;

      /**
       * @brief Current modulation depth for the cutoff sweep.
       */
      poly_float mod_depth_;

      /**
       * @brief Current phase offset for stereo modulation.
       */
      poly_float phase_offset_;

      /**
       * @brief The phaser's LFO phase, stored as an integer for fractional increments.
       */
      poly_int phase_;

      JUCE_LEAK_DETECTOR(Phaser)
  };
} // namespace vital

