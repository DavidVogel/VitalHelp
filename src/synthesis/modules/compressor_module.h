#pragma once

#include "synth_constants.h"
#include "synth_module.h"

namespace vital {

    class MultibandCompressor;

    /**
     * @brief A multiband compressor module that splits the input signal into multiple bands, applies compression to each, and then recombines them.
     *
     * The CompressorModule uses a MultibandCompressor to process the incoming audio signal.
     * It provides controls for attack, release, thresholds, ratios, gains, and mix, as well as
     * the ability to enable or disable specific frequency bands. The output includes mean squared
     * levels for input and output of each band for analysis or display.
     */
    class CompressorModule : public SynthModule {
    public:
        /**
         * @brief The outputs provided by the CompressorModule.
         *
         * - kAudio: The processed audio output (combined multiband signal).
         * - kLowInputMeanSquared: Mean squared level of the low band's input signal.
         * - kBandInputMeanSquared: Mean squared level of the mid band's input signal.
         * - kHighInputMeanSquared: Mean squared level of the high band's input signal.
         * - kLowOutputMeanSquared: Mean squared level of the low band's compressed output signal.
         * - kBandOutputMeanSquared: Mean squared level of the mid band's compressed output signal.
         * - kHighOutputMeanSquared: Mean squared level of the high band's compressed output signal.
         */
        enum {
            kAudio,
            kLowInputMeanSquared,
            kBandInputMeanSquared,
            kHighInputMeanSquared,
            kLowOutputMeanSquared,
            kBandOutputMeanSquared,
            kHighOutputMeanSquared,
            kNumOutputs
        };

        /**
         * @brief Constructs a CompressorModule, initializing outputs and internal variables.
         */
        CompressorModule();

        /**
         * @brief Destroys the CompressorModule and releases its resources.
         */
        virtual ~CompressorModule();

        /**
         * @brief Initializes the compressor module and sets up internal parameters and controls.
         *
         * This creates the internal MultibandCompressor and links various parameters from the modulation system.
         */
        virtual void init() override;

        /**
         * @brief Sets the sample rate for the compressor to ensure time-based parameters (attack, release) are correct.
         *
         * @param sample_rate The new sample rate in Hz.
         */
        virtual void setSampleRate(int sample_rate) override;

        /**
         * @brief Processes a block of samples through the compressor with the given input.
         *
         * @param audio_in    Pointer to the input audio buffer.
         * @param num_samples The number of samples in the buffer.
         */
        virtual void processWithInput(const poly_float* audio_in, int num_samples) override;

        /**
         * @brief Enables or disables the compressor module.
         *
         * When disabled, it resets internal states to ensure a clean start when re-enabled.
         *
         * @param enable True to enable, false to disable.
         */
        virtual void enable(bool enable) override;

        /**
         * @brief Performs a hard reset of the compressor, clearing buffers and resetting all states.
         */
        virtual void hardReset() override;

        /**
         * @brief Creates a clone of this CompressorModule with the same configuration.
         *
         * @return A pointer to the newly cloned CompressorModule instance.
         */
        virtual Processor* clone() const override { return new CompressorModule(*this); }

    protected:
        MultibandCompressor* compressor_; ///< The internal multiband compressor processor.

        JUCE_LEAK_DETECTOR(CompressorModule)
    };
} // namespace vital
