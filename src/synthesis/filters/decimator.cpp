#include "decimator.h"

#include "iir_halfband_decimator.h"

namespace vital {

  /**
   * @brief Constructs a Decimator with up to @p max_stages halfband decimator stages.
   *
   * Each stage is created and added as a Processor but is initially disabled until needed.
   *
   * @param max_stages The maximum number of halfband stages.
   */
  Decimator::Decimator(int max_stages) : ProcessorRouter(kNumInputs, 1), max_stages_(max_stages) {
    num_stages_ = -1;
    for (int i = 0; i < max_stages_; ++i) {
      IirHalfbandDecimator* stage = new IirHalfbandDecimator();
      stage->setOversampleAmount(1 << (max_stages_ - i - 1));  // e.g., 4x, 2x, etc.
      addProcessor(stage);
      stages_.push_back(stage);
    }
  }

  /**
   * @brief Destructor. Automatically cleans up halfband decimator stages.
   */
  Decimator::~Decimator() { }

  /**
   * @brief Initializes the Decimator by connecting stages in sequence.
   *
   * The first stage reads from the kAudio input, each subsequent stage plugs the previous stage,
   * and all eventually output to the Decimator’s main output.
   */
  void Decimator::init() {
    stages_[0]->useInput(input(kAudio));
    stages_[0]->useOutput(output());
    for (int i = 1; i < max_stages_; ++i) {
      stages_[i]->plug(stages_[i - 1], IirHalfbandDecimator::kAudio);
      stages_[i]->useOutput(output());
    }
  }

  /**
   * @brief Resets all decimator stages for the specified voices.
   *
   * @param reset_mask A poly_mask specifying which voices should be reset.
   */
  void Decimator::reset(poly_mask reset_mask) {
    for (int i = 0; i < max_stages_; ++i)
      stages_[i]->reset(reset_mask);
  }

  /**
   * @brief Processes audio by determining how many stages are needed and enabling them.
   *
   * Calculates the ratio of input sample rate to output sample rate, enabling as many
   * halfband decimator stages as needed to reach the final sample rate. Disables any
   * unused stages.
   *
   * @param num_samples The block size to process.
   */
  void Decimator::process(int num_samples) {
    int num_stages = 0;

    // Determine how many decimation stages are needed based on sample rates
    if (input(kAudio)->source->owner) {
      int input_sample_rate = input(kAudio)->source->owner->getSampleRate();
      int output_sample_rate = getSampleRate();
      while(input_sample_rate > output_sample_rate) {
        num_stages++;
        input_sample_rate /= 2;
      }

      VITAL_ASSERT(num_stages <= max_stages_);
      VITAL_ASSERT(input_sample_rate == output_sample_rate);
    }

    // If no decimation is needed, pass input directly to output
    if (num_stages == 0) {
      utils::copyBuffer(output()->buffer, input(kAudio)->source->buffer, num_samples);
      return;
    }

    // If the number of required stages has changed, reset and reconfigure them
    if (num_stages != num_stages_) {
      for (int i = 0; i < num_stages; ++i)
        stages_[i]->reset(constants::kFullMask);

      num_stages_ = num_stages;

      // Enable or disable each stage accordingly
      for (int i = 0; i < max_stages_; ++i) {
        IirHalfbandDecimator* stage = stages_[i];
        bool should_enable = i < num_stages;
        stage->enable(should_enable);
        stage->setSharpCutoff(i == num_stages - 1);

        if (should_enable) {
          int oversample_amount = 1 << (num_stages - i - 1);
          stage->setOversampleAmount(oversample_amount);
        }
      }
    }

    // Execute standard ProcessorRouter processing, which processes all sub-processors in order
    ProcessorRouter::process(num_samples);
  }
} // namespace vital
