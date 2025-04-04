#pragma once

#include "synth_filter.h"

#include "formant_manager.h"

namespace vital {
  class DigitalSvf;

  /**
   * @class FormantFilter
   * @brief A multi-formant filter for vocal/voicing effects in the Vital synthesizer.
   *
   * FormantFilter manages multiple SVF (state variable filter) instances (via FormantManager)
   * tuned to specific formant characteristics. It allows different formant styles and interpolation
   * of formant parameters based on user input (e.g., formant blending across a grid).
   */
  class FormantFilter : public ProcessorRouter, public SynthFilter {
    public:
      /**
       * @enum FormantStyle
       * @brief Different modes for the formant filter.
       */
      enum FormantStyle {
        kAOIE,            ///< Style 1: Typically blending vowels A, O, I, E
        kAIUO,            ///< Style 2: Typically blending vowels A, I, U, O
        kNumFormantStyles,///< Number of primary formant styles
        kVocalTract = kNumFormantStyles, ///< Extended style: Vocal tract modeling
        kTotalFormantFilters             ///< Total number of formant filter modes
      };

      /**
       * @brief MIDI note value used as a reference for certain interpolation/spread calculations.
       */
      static constexpr float kCenterMidi = 80.0f;

      /**
       * @brief Constructs a FormantFilter with an optional style index.
       * @param style The style index (default is 0).
       */
      FormantFilter(int style = 0);

      /**
       * @brief Virtual destructor.
       */
      virtual ~FormantFilter() { }

      /**
       * @brief Resets internal states for the voices specified by the reset mask.
       * @param reset_mask A bitmask indicating which voices to reset.
       */
      void reset(poly_mask reset_mask) override;

      /**
       * @brief Performs a hard reset of all internal states and parameters.
       */
      void hardReset() override;

      /**
       * @brief Initializes the formant filter processors (called after construction).
       *
       * Creates and connects the BilinearInterpolate, Interpolate, and Add/Multiply nodes
       * needed to produce the final formant filter stages.
       */
      void init() override;

      /**
       * @brief Creates a deep copy (clone) of this FormantFilter instance.
       * @return A pointer to the newly cloned FormantFilter.
       */
      virtual Processor* clone() const override { return new FormantFilter(*this); }

      /**
       * @brief Configures this FormantFilter (and associated DigitalSvf instances) from a FilterState.
       * @param filter_state The FilterState containing cutoff, resonance, and other parameters.
       */
      void setupFilter(const FilterState& filter_state) override;

      /**
       * @brief Gets a pointer to the underlying DigitalSvf filter for a specific formant index.
       * @param index The index of the formant (0-based).
       * @return A pointer to the DigitalSvf used by this formant.
       */
      DigitalSvf* getFormant(int index) { return formant_manager_->getFormant(index); }

    protected:
      /**
       * @brief Manages a set of DigitalSvf objects, each corresponding to a single formant.
       */
      FormantManager* formant_manager_;

      /**
       * @brief Current style index (e.g., kAOIE, kAIUO, etc.).
       */
      int style_;

      JUCE_LEAK_DETECTOR(FormantFilter)
  };
} // namespace vital
