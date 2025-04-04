#pragma once

#include "processor.h"
#include "synth_constants.h"

namespace vital {

  /**
   * @class IirHalfbandDecimator
   * @brief An IIR-based half-band decimator for downsampling audio by a factor of 2.
   *
   * This decimator uses an IIR half-band filter approach where each pair of input samples
   * is combined into a single output sample. The filter can operate in two modes:
   * - A faster, lighter mode using 9 taps.
   * - A sharper cutoff mode using 25 taps.
   */
  class IirHalfbandDecimator : public Processor {
    public:
      /**
       * @brief Number of taps in the lighter (9-tap) filter mode.
       */
      static constexpr int kNumTaps9 = 2;

      /**
       * @brief Number of taps in the sharper (25-tap) filter mode.
       */
      static constexpr int kNumTaps25 = 6;

      /**
       * @brief Coefficients for the 9-tap IIR half-band filter, stored as pairs of poly_float.
       */
      static poly_float kTaps9[kNumTaps9];

      /**
       * @brief Coefficients for the 25-tap IIR half-band filter, stored as pairs of poly_float.
       */
      static poly_float kTaps25[kNumTaps25];

      /**
       * @enum InputIndices
       * @brief Enum for decimator input indices.
       */
      enum {
        kAudio,     ///< Main audio input for decimation.
        kNumInputs  ///< Total number of inputs for this processor.
      };

      /**
       * @brief Constructs an IirHalfbandDecimator and initializes its memory.
       */
      IirHalfbandDecimator();

      /**
       * @brief Virtual destructor.
       */
      virtual ~IirHalfbandDecimator() { }

      /**
       * @brief Cloning is not supported for this processor.
       * @return Returns nullptr, with an assertion failure.
       */
      virtual Processor* clone() const override {
        VITAL_ASSERT(false);
        return nullptr;
      }

      /**
       * @brief Processes audio data by decimating it (halving the sample rate).
       * @param num_samples The number of output samples to process (input must have 2 * num_samples).
       */
      virtual void process(int num_samples) override;

      /**
       * @brief Resets the filter states (delay lines) for the specified voices.
       * @param reset_mask A bitmask specifying which voices to reset.
       */
      void reset(poly_mask reset_mask) override;

      /**
       * @brief Enables or disables the sharper 25-tap cutoff mode.
       * @param sharp_cutoff If true, use 25 taps; if false, use 9 taps.
       */
      force_inline void setSharpCutoff(bool sharp_cutoff) { sharp_cutoff_ = sharp_cutoff; }

    private:
      /**
       * @brief Flag indicating whether to use the sharper 25-tap filter (true) or the 9-tap filter (false).
       */
      bool sharp_cutoff_;

      /**
       * @brief IIR delay line memory for the input path.
       *
       * In the 9-tap mode, only a subset of these entries are used.
       */
      poly_float in_memory_[kNumTaps25];

      /**
       * @brief IIR delay line memory for the output path.
       *
       * In the 9-tap mode, only a subset of these entries are used.
       */
      poly_float out_memory_[kNumTaps25];

      JUCE_LEAK_DETECTOR(IirHalfbandDecimator)
  };
} // namespace vital
