#pragma once

#include "processor.h"
#include "synth_filter.h"

#include "one_pole_filter.h"
#include "futils.h"

namespace vital {

  /**
   * @class LadderFilter
   * @brief A classic transistor ladder-style filter for the Vital synthesizer.
   *
   * The LadderFilter class simulates a multi-stage (four-pole) ladder filter with drive,
   * resonance, and various output slopes (12 dB, 24 dB, etc.). It supports different styles
   * and pass blends for flexible filter curves.
   */
  class LadderFilter : public Processor, public SynthFilter {
    public:
      /**
       * @brief Number of filter stages in the ladder (4-pole ladder).
       */
      static constexpr int kNumStages = 4;

      /**
       * @brief Resonance tuning factor to align the filter’s internal response with musical expectations.
       */
      static constexpr mono_float kResonanceTuning = 1.66f;

      /**
       * @brief Minimum resonance value.
       */
      static constexpr mono_float kMinResonance = 0.001f;

      /**
       * @brief Maximum resonance value.
       */
      static constexpr mono_float kMaxResonance = 4.1f;

      /**
       * @brief Maximum main filter coefficient value (clamps the cutoff).
       */
      static constexpr mono_float kMaxCoefficient = 0.35f;

      /**
       * @brief Boost factor added to the resonance based on drive.
       */
      static constexpr mono_float kDriveResonanceBoost = 5.0f;

      /**
       * @brief Minimum cutoff frequency in Hz (used internally).
       */
      static constexpr mono_float kMinCutoff = 1.0f;

      /**
       * @brief Maximum cutoff frequency in Hz (used internally).
       */
      static constexpr mono_float kMaxCutoff = 20000.0f;

      /**
       * @brief Constructs a new LadderFilter.
       */
      LadderFilter();

      /**
       * @brief Destructor for the LadderFilter.
       */
      virtual ~LadderFilter() { }

      /**
       * @brief Creates a clone (deep copy) of this LadderFilter instance.
       * @return A pointer to the cloned LadderFilter.
       */
      virtual Processor* clone() const override { return new LadderFilter(*this); }

      /**
       * @brief Processes the input audio buffer through this ladder filter.
       * @param num_samples The number of samples to process.
       *
       * Reads from the input buffer, applies the ladder filter stages, and writes the
       * results to the output buffer.
       */
      virtual void process(int num_samples) override;

      /**
       * @brief Configures the filter parameters based on a FilterState.
       * @param filter_state The FilterState containing cutoff, resonance, style, etc.
       */
      void setupFilter(const FilterState& filter_state) override;

      /**
       * @brief Processes a single sample through the ladder filter stages.
       * @param audio_in The input audio sample.
       * @param coefficient The main filter coefficient, related to cutoff.
       * @param resonance The current resonance value.
       * @param drive The current drive amount.
       *
       * Internally updates the filter states in each of the four poles with saturation.
       */
      force_inline void tick(poly_float audio_in, poly_float coefficient,
                             poly_float resonance, poly_float drive);

      /**
       * @brief Resets internal states of each filter stage according to the given mask.
       * @param reset_mask A bitmask indicating which voices need resetting.
       */
      void reset(poly_mask reset_mask) override;

      /**
       * @brief Performs a hard reset of all filter states.
       */
      void hardReset() override;

      /**
       * @brief Retrieves the current drive setting.
       * @return The computed drive amount for this ladder filter.
       */
      poly_float getDrive() { return drive_; }

      /**
       * @brief Retrieves the current resonance setting.
       * @return The computed resonance value for this ladder filter.
       */
      poly_float getResonance() { return resonance_; }

      /**
       * @brief Retrieves the scale value for one of the filter’s output stages.
       * @param index Index of the stage scale (0 through kNumStages).
       * @return The scaling factor used for that stage output in the final mix.
       */
      poly_float getStageScale(int index) { return stage_scales_[index]; }

    private:
      /**
       * @brief Updates the internal stage scales based on the style and pass blend.
       * @param filter_state The FilterState containing style, pass blend, etc.
       */
      void setStageScales(const FilterState& filter_state);

      /**
       * @brief The computed resonance value for this filter instance.
       */
      poly_float resonance_;

      /**
       * @brief The computed drive factor for this filter instance.
       */
      poly_float drive_;

      /**
       * @brief A post-multiply normalization factor to manage output levels after drive.
       */
      poly_float post_multiply_;

      /**
       * @brief Scaling factors for each stage’s output in the final filter sum.
       *
       * The ladder filter can produce different slope outputs (e.g., 24 dB, 12 dB), so each stage
       * can be weighted differently to achieve the desired response.
       */
      poly_float stage_scales_[kNumStages + 1];

      /**
       * @brief Four one-pole filter stages with non-linear saturation.
       */
      OnePoleFilter<futils::algebraicSat> stages_[kNumStages];

      /**
       * @brief Temporary storage of the input sample for usage in tick().
       */
      poly_float filter_input_;

      JUCE_LEAK_DETECTOR(LadderFilter)
  };
} // namespace vital
