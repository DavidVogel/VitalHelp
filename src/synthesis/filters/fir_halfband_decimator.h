#pragma once

#include "processor.h"
#include "synth_constants.h"

namespace vital {

  /**
   * @class FirHalfbandDecimator
   * @brief A FIR half-band decimator for downsampling audio by a factor of 2.
   *
   * This decimator uses a half-band FIR filter to combine every pair of input samples
   * into a single output sample, reducing the sample rate by half.
   */
  class FirHalfbandDecimator : public Processor {
    public:
      /**
       * @brief Number of FIR taps in the filter.
       */
      static constexpr int kNumTaps = 32;

      /**
       * @enum InputIndices
       * @brief Enum representing the processor's input indices.
       */
      enum {
        kAudio,     ///< The main audio input.
        kNumInputs  ///< Total number of inputs for this processor.
      };

      /**
       * @brief Constructs a FirHalfbandDecimator and initializes taps.
       *
       * Sets up the filter coefficients and resets the internal memory.
       */
      FirHalfbandDecimator();

      /**
       * @brief Virtual destructor.
       */
      virtual ~FirHalfbandDecimator() { }

      /**
       * @brief Clones (deep copies) this decimator.
       * @return A pointer to a new FirHalfbandDecimator instance.
       */
      virtual Processor* clone() const override { return new FirHalfbandDecimator(*this); }

      /**
       * @brief Saves the last few samples of audio to memory, preparing for the next processing block.
       * @param num_samples The number of samples that were processed in this block (i.e., half the input buffer size).
       *
       * This function stores samples that will be needed by subsequent processing blocks.
       */
      void saveMemory(int num_samples);

      /**
       * @brief Processes the input audio by decimating it, producing half the number of output samples.
       * @param num_samples The number of output samples to produce. (The input must be 2 * num_samples in length.)
       */
      virtual void process(int num_samples) override;

    private:
      /**
       * @brief Resets the internal buffer memory for all voices indicated by the reset mask.
       * @param reset_mask A bitmask indicating which voices to reset.
       */
      void reset(poly_mask reset_mask) override;

      /**
       * @brief A small array holding some of the most recent input samples for continuity between blocks.
       */
      poly_float memory_[kNumTaps / 2 - 1];

      /**
       * @brief The FIR filter coefficients, stored in a poly_float for pairwise processing.
       */
      poly_float taps_[kNumTaps / 2];

      JUCE_LEAK_DETECTOR(FirHalfbandDecimator)
  };
} // namespace vital
