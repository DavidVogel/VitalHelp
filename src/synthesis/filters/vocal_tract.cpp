#include "vocal_tract.h"

namespace vital {

  /**
   * @brief Constructs a VocalTract processor with one output channel.
   */
  VocalTract::VocalTract() : ProcessorRouter(kNumInputs, 1) {
  }

  /**
   * @brief Destructor for the VocalTract class.
   */
  VocalTract::~VocalTract() {
  }

  /**
   * @brief Resets the VocalTract’s internal states for the specified voices.
   * @param reset_mask A bitmask specifying which voices to reset.
   */
  void VocalTract::reset(poly_mask reset_mask) {
    // Currently no internal states to reset.
  }

  /**
   * @brief Performs a full reset, clearing all internal states.
   */
  void VocalTract::hardReset() {
    reset(constants::kFullMask);
  }

  /**
   * @brief Reads from the audio input and calls processWithInput() for processing.
   * @param num_samples The number of samples to process.
   */
  void VocalTract::process(int num_samples) {
    const poly_float* audio_in = input(kAudio)->source->buffer;
    processWithInput(audio_in, num_samples);
  }

  /**
   * @brief Processes the given audio input buffer, but no operation is currently implemented.
   * @param audio_in Pointer to the input samples.
   * @param num_samples Number of samples to process.
   */
  void VocalTract::processWithInput(const poly_float* audio_in, int num_samples) {
    // Implementation placeholder: any actual shaping or vocal tract modeling
    // logic would occur here, modifying the signal and writing it to output().
  }

} // namespace vital
