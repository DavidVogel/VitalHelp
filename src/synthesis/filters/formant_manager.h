#pragma once

#include "processor_router.h"
#include "synth_constants.h"

namespace vital {

  class DigitalSvf;

  /**
   * @class FormantManager
   * @brief Manages a collection of DigitalSvf instances for formant filtering.
   *
   * The FormantManager class holds multiple DigitalSvf objects, each of which can be set to
   * represent a different vowel formant or resonance characteristic. It sums the outputs of these
   * formants to produce a single audio output.
   */
  class FormantManager : public ProcessorRouter {
    public:
      /**
       * @brief Minimum allowed resonance for each DigitalSvf in this manager.
       */
      static constexpr mono_float kMinResonance = 4.0f;

      /**
       * @brief Maximum allowed resonance for each DigitalSvf in this manager.
       */
      static constexpr mono_float kMaxResonance = 30.0f;

      /**
       * @brief Constructs a FormantManager to hold a given number of DigitalSvf formants.
       * @param num_formants Number of formant filters (DigitalSvf objects) to create.
       */
      FormantManager(int num_formants = 4);

      /**
       * @brief Virtual destructor.
       */
      virtual ~FormantManager() { }

      /**
       * @brief Initializes the internal routing of formant filters, creating the summing processor.
       *
       * Called after construction (and any additions of DigitalSvf objects). Sets up how the formant
       * outputs are summed into a single output.
       */
      virtual void init() override;

      /**
       * @brief Resets stateful data (e.g., filter history) in each DigitalSvf for voices indicated by the reset_mask.
       * @param reset_mask A bitmask indicating which voices need resetting.
       */
      void reset(poly_mask reset_mask) override;

      /**
       * @brief Fully resets all formants (for all voices).
       *
       * Clears any internal filter states, so subsequent audio processing starts from a blank slate.
       */
      void hardReset() override;

      /**
       * @brief Clones (deep copies) this FormantManager.
       * @return A pointer to a newly allocated FormantManager.
       */
      virtual Processor* clone() const override {
        return new FormantManager(*this);
      }

      /**
       * @brief Returns a pointer to the requested formant filter (DigitalSvf).
       * @param index Zero-based index of the requested formant.
       * @return A pointer to the DigitalSvf at the specified index.
       */
      DigitalSvf* getFormant(int index = 0) { return formants_[index]; }

      /**
       * @brief Gets the total number of formants managed by this object.
       * @return The number of DigitalSvf objects contained here.
       */
      int numFormants() { return static_cast<int>(formants_.size()); }

    protected:
      /**
       * @brief A list of DigitalSvf pointers, each representing a vowel formant or similar resonant filter.
       */
      std::vector<DigitalSvf*> formants_;

      JUCE_LEAK_DETECTOR(FormantManager)
  };
} // namespace vital
