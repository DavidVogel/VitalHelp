#pragma once

#include "processor.h"
#include "synth_constants.h"

namespace vital {

  /**
   * @class LinkwitzRileyFilter
   * @brief A Linkwitz-Riley crossover filter splitting audio into low and high bands.
   *
   * This filter generates two outputs: a low-pass output (kAudioLow) and
   * a high-pass output (kAudioHigh). Internally, it uses cascaded biquad sections
   * for both low-pass and high-pass. The cutoff frequency is set via the constructor,
   * and recalculated whenever the sample rate or oversampling changes.
   */
  class LinkwitzRileyFilter : public Processor {
    public:
      /**
       * @enum InputIndices
       * @brief Enum for the filter input indices.
       */
      enum {
        kAudio,    ///< Main audio input.
        kNumInputs ///< Total number of inputs.
      };

      /**
       * @enum OutputIndices
       * @brief Enum for the filter output indices.
       */
      enum {
        kAudioLow,  ///< Low-frequency output.
        kAudioHigh, ///< High-frequency output.
        kNumOutputs ///< Total number of outputs.
      };

      /**
       * @brief Constructs a LinkwitzRileyFilter with the given cutoff frequency.
       * @param cutoff The cutoff frequency in Hz.
       */
      LinkwitzRileyFilter(mono_float cutoff);

      /**
       * @brief Virtual destructor.
       */
      virtual ~LinkwitzRileyFilter() { }

      /**
       * @brief Creates a clone of this LinkwitzRileyFilter.
       * @return Pointer to the new LinkwitzRileyFilter.
       */
      virtual Processor* clone() const override { return new LinkwitzRileyFilter(*this); }

      /**
       * @brief Processes the input audio with the specified number of samples.
       * @param num_samples Number of samples to process.
       *
       * This will split the input audio into two outputs (low and high).
       */
      virtual void process(int num_samples) override;

      /**
       * @brief Processes the provided audio input buffer and writes
       *        low and high outputs to the Processor’s buffers.
       * @param audio_in Pointer to the input audio buffer.
       * @param num_samples Number of samples to process.
       */
      void processWithInput(const poly_float* audio_in, int num_samples) override;

      /**
       * @brief Computes and updates the biquad coefficients based on the current cutoff and sample rate.
       */
      void computeCoefficients();

      /**
       * @brief Sets the internal sample rate and recomputes filter coefficients.
       * @param sample_rate The new sample rate to use.
       */
      void setSampleRate(int sample_rate) override;

      /**
       * @brief Sets the internal oversample amount and recomputes filter coefficients.
       * @param oversample_amount The new oversampling factor.
       */
      void setOversampleAmount(int oversample_amount) override;

      /**
       * @brief Resets the internal states (delay lines) for the specified voices.
       * @param reset_mask A bitmask specifying which voices to reset.
       */
      void reset(poly_mask reset_mask) override;

    private:
      /**
       * @brief The cutoff frequency for the filter, in Hz.
       */
      mono_float cutoff_;

      // Biquad Coefficients.
      mono_float low_in_0_, low_in_1_, low_in_2_;
      mono_float low_out_1_, low_out_2_;
      mono_float high_in_0_, high_in_1_, high_in_2_;
      mono_float high_out_1_, high_out_2_;

      /**
       * @brief Past input and output values for each of the filter’s internal stages (2 cascades each for low and high).
       *
       * The arrays are indexed by `[kAudioLow or kAudioHigh]` for the first dimension,
       * and by the stage dimension (e.g., 1a, 2a, 1b, 2b) in each array.
       */
      poly_float past_in_1a_[kNumOutputs], past_in_2a_[2 * kNumOutputs];
      poly_float past_out_1a_[2 * kNumOutputs], past_out_2a_[2 * kNumOutputs];
      poly_float past_in_1b_[kNumOutputs], past_in_2b_[2 * kNumOutputs];
      poly_float past_out_1b_[2 * kNumOutputs], past_out_2b_[2 * kNumOutputs];

      JUCE_LEAK_DETECTOR(LinkwitzRileyFilter)
  };
} // namespace vital
