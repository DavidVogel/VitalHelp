#pragma once

#include "processor_router.h"
#include "synth_constants.h"

namespace vital {

    /**
     * @class Upsampler
     * @brief A simple upsampler that duplicates samples to increase the sample rate by an integral factor.
     *
     * The Upsampler takes an input signal and increases its effective sample rate by repeating each input sample
     * a specified number of times (given by the oversampling factor). It does not apply any filtering or interpolation,
     * simply repeating samples to form the output. While this is not a high-quality resampling method and may
     * introduce aliasing, it is computationally efficient and can be useful in certain contexts where oversampling
     * is desired before further processing.
     *
     * Inputs:
     * - kAudio: The input audio signal to be upsampled.
     *
     * Output:
     * - The upsampled audio signal. Each input sample is replicated oversample_amount times.
     */
    class Upsampler : public ProcessorRouter {
    public:
        /// Input indices.
        enum {
            kAudio,    ///< Input audio signal.
            kNumInputs
        };

        /**
         * @brief Constructs an Upsampler.
         *
         * Initializes the ProcessorRouter with one input and one output. No additional configuration is needed.
         */
        Upsampler();

        /**
         * @brief Destructor.
         */
        virtual ~Upsampler();

        Processor* clone() const override { VITAL_ASSERT(false); return nullptr; }

        /**
         * @brief Processes a block of audio samples using the already connected input.
         *
         * @param num_samples The number of samples in the output buffer. Since the input buffer
         *                    will have fewer samples (input_samples = output_samples / oversample_factor),
         *                    it copies each input sample multiple times.
         */
        virtual void process(int num_samples) override;

        /**
         * @brief Processes a given block of input samples by upsampling them.
         *
         * Copies each input sample oversample_amount times to produce the upsampled output.
         *
         * @param audio_in Pointer to the input audio buffer.
         * @param num_samples The number of output samples to produce.
         */
        virtual void processWithInput(const poly_float* audio_in, int num_samples) override;

    private:
        JUCE_LEAK_DETECTOR(Upsampler)
    };
} // namespace vital
