#pragma once

#include "processor.h"
#include "synth_constants.h"

namespace vital {

  /**
   * @class DcFilter
   * @brief A simple DC blocking filter implemented as a one-pole high-pass filter.
   *
   * The DcFilter removes DC offset from the audio signal by subtracting a filtered version
   * of the previous sample. This is useful for preventing drift or high offsets in certain
   * synthesizer processes.
   */
  class DcFilter : public Processor {
    public:
      /**
       * @brief A constant used to compute the one-pole filter’s coefficient from the sample rate.
       *
       * Typically 1.0, meaning the filter is fully dependent on the ratio
       * (1.0f - 1.0f / currentSampleRate).
       */
      static constexpr mono_float kCoefficientToSrConstant = 1.0f;

      /**
       * @enum InputIndices
       * @brief Enumerates input indices for this DcFilter Processor.
       */
      enum {
        kAudio, ///< Audio input buffer.
        kReset, ///< Reset trigger input.
        kNumInputs
      };

      /**
       * @brief Constructs a DcFilter Processor with default parameters.
       */
      DcFilter();

      /** @brief Default destructor. */
      virtual ~DcFilter() { }

      /**
       * @brief Creates a clone of this DcFilter via copy constructor.
       * @return A new DcFilter object cloned from this one.
       */
      virtual Processor* clone() const override { return new DcFilter(*this); }

      /**
       * @brief Processes a block of samples, pulling from the kAudio input.
       *
       * @param num_samples Number of samples in the buffer.
       */
      virtual void process(int num_samples) override;

      /**
       * @brief Processes a block of samples using the provided input buffer.
       *
       * @param audio_in    Pointer to the audio input buffer.
       * @param num_samples Number of samples to process.
       */
      virtual void processWithInput(const poly_float* audio_in, int num_samples) override;

      /**
       * @brief Sets the sample rate for this filter, recalculating the filter’s coefficient.
       *
       * @param sample_rate The new sample rate in Hz.
       */
      void setSampleRate(int sample_rate) override;

      /**
       * @brief Processes a single sample pair (input to output).
       *
       * @param audio_in  The current input sample.
       * @param audio_out Reference to the output sample location.
       */
      void tick(const poly_float& audio_in, poly_float& audio_out);

    private:
      /**
       * @brief Resets the filter’s internal states for the voices indicated by the reset mask.
       *
       * @param reset_mask A poly_mask specifying which voices should be reset.
       */
      void reset(poly_mask reset_mask) override;

      /**
       * @brief One-pole high-pass filter coefficient used to remove DC offset.
       */
      mono_float coefficient_;

      /**
       * @brief Stores the previous input sample (for each voice).
       */
      poly_float past_in_;

      /**
       * @brief Stores the previous output sample (for each voice).
       */
      poly_float past_out_;

      JUCE_LEAK_DETECTOR(DcFilter)
  };
} // namespace vital
