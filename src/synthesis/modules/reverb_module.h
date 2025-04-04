#pragma once

#include "synth_constants.h"
#include "synth_module.h"

namespace vital {

    class Reverb;

    /**
     * @brief A module that applies a reverb effect to an audio signal.
     *
     * The ReverbModule manages a Reverb processor, providing parameters for decay time,
     * filtering, chorus modulation within the reverb tail, and dry/wet mix. It integrates
     * into the synthesis graph as a SynthModule.
     */
    class ReverbModule : public SynthModule {
    public:
        /**
         * @brief Constructs a ReverbModule.
         *
         * Initializes the module with one output and no direct inputs,
         * to be fed by the processWithInput function.
         */
        ReverbModule();

        /**
         * @brief Destructor for ReverbModule.
         */
        virtual ~ReverbModule();

        /**
         * @brief Initializes the reverb module and sets up parameter controls.
         *
         * Creates controls for various reverb parameters and plugs them into the internal Reverb processor.
         */
        void init() override;

        /**
         * @brief Performs a hard reset of the reverb.
         *
         * Resets the internal state of the reverb so it starts from a clean slate.
         */
        void hardReset() override;

        /**
         * @brief Enables or disables the reverb module.
         *
         * When disabling, the reverb state is reset. When enabling, reverb processing resumes.
         *
         * @param enable True to enable, False to disable.
         */
        void enable(bool enable) override;

        /**
         * @brief Sets the sample rate of the reverb processor.
         *
         * Adjusts internal delay lines and other sample-rate-dependent parameters.
         *
         * @param sample_rate The sample rate in Hz.
         */
        void setSampleRate(int sample_rate) override;

        /**
         * @brief Processes an input audio buffer through the reverb effect.
         *
         * @param audio_in Pointer to the input audio samples.
         * @param num_samples The number of samples to process.
         */
        void processWithInput(const poly_float* audio_in, int num_samples) override;

        /**
         * @brief Clones the reverb module.
         *
         * @return A new ReverbModule identical to the current one.
         */
        Processor* clone() const override { return new ReverbModule(*this); }

    protected:
        Reverb* reverb_; /**< The internal reverb processor instance. */

        JUCE_LEAK_DETECTOR(ReverbModule)
    };
} // namespace vital
