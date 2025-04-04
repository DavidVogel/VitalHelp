#pragma once

#include "processor_router.h"
#include "synth_constants.h"

namespace vital {

  // Forward declaration of IirHalfbandDecimator
  class IirHalfbandDecimator;

  /**
   * @class Decimator
   * @brief A ProcessorRouter that intelligently reduces audio sample rate based on configured stages.
   *
   * The Decimator manages multiple IirHalfbandDecimator stages, enabling them dynamically
   * depending on the input sample rate relative to the output sample rate. Each stage
   * halves the sample rate if needed.
   */
  class Decimator : public ProcessorRouter {
    public:
      /**
       * @enum InputIndices
       * @brief Enumerates the input indices for this Decimator.
       */
      enum {
        kAudio,   ///< The main audio input to be decimated.
        kNumInputs
      };

      /**
       * @brief Constructs a Decimator with a specified maximum number of halfband stages.
       *
       * @param max_stages The maximum number of stages for sample rate halving.
       */
      Decimator(int max_stages = 1);

      /**
       * @brief Destructor. Cleans up decimator stages.
       */
      virtual ~Decimator();

      /**
       * @brief Initializes the Decimator, hooking up audio connections for each stage.
       */
      void init() override;

      /**
       * @brief Resets all decimator stages for specified voices.
       *
       * @param reset_mask A poly_mask specifying which voices should be reset.
       */
      void reset(poly_mask reset_mask) override;

      /**
       * @brief Cloning not implemented for the Decimator.
       * @return Always returns nullptr.
       */
      virtual Processor* clone() const override { VITAL_ASSERT(false); return nullptr; }

      /**
       * @brief Main audio processing routine that checks required decimation stages and processes them.
       *
       * @param num_samples The block size to be processed.
       */
      virtual void process(int num_samples) override;

      /**
       * @brief No-op for oversample amount setting, handled internally.
       *
       * @param oversample The new oversampling factor (unused here).
       */
      virtual void setOversampleAmount(int) override { }

    private:
      /**
       * @brief The currently active number of decimation stages.
       */
      int num_stages_;

      /**
       * @brief The maximum number of decimation stages permitted.
       */
      int max_stages_;

      /**
       * @brief Container of pointers to IirHalfbandDecimator stages, one per possible decimation stage.
       */
      std::vector<IirHalfbandDecimator*> stages_;

      JUCE_LEAK_DETECTOR(Decimator)
  };
} // namespace vital
