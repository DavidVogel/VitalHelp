#pragma once

#include "processor.h"
#include "synth_filter.h"

#include "futils.h"
#include "one_pole_filter.h"

namespace vital {

  /**
   * @class DiodeFilter
   * @brief A diode ladder filter implementation for the Vital synthesizer.
   *
   * This class inherits from Processor and SynthFilter, providing a nonlinear diode ladder
   * filter stage with high-pass filtering options and drive/resonance controls. It uses a
   * combination of one-pole filters and saturation functions to emulate the diode ladder
   * behavior.
   */
  class DiodeFilter : public Processor, public SynthFilter {
    public:
      /**
       * @brief Minimum resonance value for the diode filter.
       */
      static constexpr mono_float kMinResonance = 0.7f;

      /**
       * @brief Maximum resonance value for the diode filter.
       */
      static constexpr mono_float kMaxResonance = 17.0f;

      /**
       * @brief Minimum cutoff frequency in Hz (used internally).
       */
      static constexpr mono_float kMinCutoff = 1.0f;

      /**
       * @brief High-pass cutoff frequency in Hz (used internally).
       */
      static constexpr mono_float kHighPassFrequency = 20.0f;

      /**
       * @brief Default constructor.
       *
       * Initializes a new instance of the DiodeFilter.
       */
      DiodeFilter();

      /**
       * @brief Virtual destructor.
       */
      virtual ~DiodeFilter() { }

      /**
       * @brief Creates a clone (deep copy) of the DiodeFilter object.
       * @return A pointer to a cloned DiodeFilter.
       */
      virtual Processor* clone() const override { return new DiodeFilter(*this); }

      /**
       * @brief Processes audio data through the diode filter.
       * @param num_samples The number of samples to process.
       *
       * This method reads the input buffer, applies the diode filter algorithm, and writes
       * the output to the output buffer.
       */
      virtual void process(int num_samples) override;

      /**
       * @brief Sets up filter parameters from the given FilterState.
       * @param filter_state A const reference to the FilterState containing the filter parameters.
       *
       * This updates internal fields such as resonance, drive, and high-pass parameters.
       */
      void setupFilter(const FilterState& filter_state) override;

      /**
       * @brief Processes a single sample of audio through the diode ladder filter stages.
       * @param audio_in The input audio sample.
       * @param coefficient The main filter coefficient (low-pass).
       * @param high_pass_ratio The precomputed ratio for the high-pass section.
       * @param high_pass_amount The blend or amount of high-pass processing.
       * @param high_pass_feedback_coefficient Coefficient for feedback filtering in the high-pass section.
       * @param resonance The current resonance (Q) setting.
       * @param drive The current drive setting.
       *
       * This function applies the diode ladder core processing, saturations, and high-pass
       * sections for a single sample.
       */
      force_inline void tick(poly_float audio_in,
                             poly_float coefficient,
                             poly_float high_pass_ratio,
                             poly_float high_pass_amount,
                             poly_float high_pass_feedback_coefficient,
                             poly_float resonance,
                             poly_float drive);

      /**
       * @brief Resets internal filter states (such as one-pole filters) with the given reset mask.
       * @param reset_mask A bitmask indicating which voices need resetting.
       *
       * This method is typically called when a new note is triggered or a parameter changes
       * drastically and the filter state needs to be reinitialized.
       */
      void reset(poly_mask reset_mask) override;

      /**
       * @brief Performs a complete hard reset of all internal states (for all voices).
       *
       * This method sets the filter back to its initial state and resets all parameter
       * values to their defaults.
       */
      void hardReset() override;

      /**
       * @brief Gets the current resonance setting of the filter.
       * @return The current resonance (Q) value.
       */
      poly_float getResonance() { return resonance_; }

      /**
       * @brief Gets the current drive setting of the filter.
       * @return The current drive amount.
       */
      poly_float getDrive() { return drive_; }

      /**
       * @brief Gets the current high-pass ratio.
       * @return The ratio value used by the high-pass filter section.
       */
      poly_float getHighPassRatio() { return high_pass_ratio_; }

      /**
       * @brief Gets the current high-pass amount.
       * @return The blend amount for the high-pass filter.
       */
      poly_float getHighPassAmount() { return high_pass_amount_; }

    private:
      /**
       * @brief The current resonance setting.
       */
      poly_float resonance_;

      /**
       * @brief The current drive setting.
       */
      poly_float drive_;

      /**
       * @brief Post-multiply factor to normalize output volume when drive changes.
       */
      poly_float post_multiply_;

      /**
       * @brief Ratio applied to the main filter coefficient for the high-pass stage.
       */
      poly_float high_pass_ratio_;

      /**
       * @brief Blend amount controlling how much of the high-pass effect is applied.
       */
      poly_float high_pass_amount_;

      /**
       * @brief Internal feedback coefficient for the high-pass filter stage.
       */
      poly_float feedback_high_pass_coefficient_;

      /**
       * @brief Saturates a value using tanh approximation.
       * @param value The input value.
       * @return The saturated value.
       */
      static force_inline poly_float saturate(poly_float value) {
        return futils::tanh(value);
      }

      /**
       * @brief Saturates a value by clamping it within [-1.0, 1.0].
       * @param value The input value.
       * @return The clamped value.
       */
      static force_inline poly_float saturate2(poly_float value) {
        return utils::clamp(value, -1.0f, 1.0f);
      }

      /**
       * @brief OnePoleFilter objects for the initial high-pass stages.
       */
      OnePoleFilter<> high_pass_1_;  ///< First stage high-pass filter.
      OnePoleFilter<> high_pass_2_;  ///< Second stage high-pass filter.

      /**
       * @brief Additional filters used in the feedback and stage processing of the diode ladder.
       */
      OnePoleFilter<> high_pass_feedback_;    ///< High-pass filter in the feedback path.
      OnePoleFilter<saturate> stage1_;        ///< First filter stage with tanh saturation.
      OnePoleFilter<> stage2_;                ///< Second filter stage.
      OnePoleFilter<> stage3_;                ///< Third filter stage.
      OnePoleFilter<saturate2> stage4_;       ///< Final filter stage with clamping saturation.

      JUCE_LEAK_DETECTOR(DiodeFilter)
  };
} // namespace vital
