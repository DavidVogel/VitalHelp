#include "upsampler.h"

namespace vital {

  /**
   * @brief Default constructor, initializes the Upsampler with one output channel.
   */
  Upsampler::Upsampler() : ProcessorRouter(kNumInputs, 1) { }

  /**
   * @brief Virtual destructor for the Upsampler class.
   */
  Upsampler::~Upsampler() { }

  /**
   * @brief Processes audio by retrieving the input buffer and passing it to processWithInput().
   * @param num_samples Number of output samples to process.
   *
   * The actual upsampling logic is handled in processWithInput(), which repeats each sample
   * oversample_amount times.
   */
  void Upsampler::process(int num_samples) {
    const poly_float* audio_in = input(kAudio)->source->buffer;
    processWithInput(audio_in, num_samples);
  }

  /**
   * @brief Performs the upsampling by repeating each input sample a specified number of times.
   * @param audio_in Pointer to the buffer containing the original audio samples.
   * @param num_samples Number of output samples to write to.
   *
   * For each input sample, we store it multiple times in the output buffer, where the number
   * of times is determined by getOversampleAmount(). This expands the sample buffer by that factor.
   */
  void Upsampler::processWithInput(const poly_float* audio_in, int num_samples) {
    poly_float* destination = output()->buffer;

    int oversample_amount = getOversampleAmount();

    for (int i = 0; i < num_samples; ++i) {
      int offset = i * oversample_amount;
      for (int s = 0; s < oversample_amount; ++s)
        destination[offset + s] = audio_in[i];
    }
  }
} // namespace vital
