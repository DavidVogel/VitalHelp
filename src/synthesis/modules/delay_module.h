#pragma once

#include "synth_constants.h"
#include "synth_module.h"

#include "delay.h"

namespace vital {

    /**
     * @brief A stereo delay effect module providing adjustable delay times, feedback, filtering, and wet/dry mix.
     *
     * The DelayModule class uses a StereoDelay processor to implement a time-based echo effect. Parameters such as delay time,
     * feedback, and mix can be modulated and can be either free-running or tempo-synced.
     */
    class DelayModule : public SynthModule {
    public:
        /// The maximum delay time in seconds.
        static constexpr mono_float kMaxDelayTime = 4.0f;

        /**
         * @brief Constructs the DelayModule, linking it to a beats-per-second output for tempo synchronization.
         *
         * @param beats_per_second An Output pointer providing the current tempo in beats per second, used for sync modes.
         */
        DelayModule(const Output* beats_per_second);

        /**
         * @brief Destroys the DelayModule and releases associated resources.
         */
        virtual ~DelayModule();

        /**
         * @brief Initializes the DelayModule, creating and linking parameters and setting up the StereoDelay processor.
         */
        virtual void init() override;

        /**
         * @brief Performs a hard reset on the delay, clearing any buffered samples and returning to initial state.
         */
        virtual void hardReset() override { delay_->hardReset(); }

        /**
         * @brief Enables or disables the DelayModule.
         *
         * When disabling, it ensures a clean state by processing a single sample and performing a hard reset.
         *
         * @param enable True to enable, false to disable.
         */
        virtual void enable(bool enable) override {
            SynthModule::enable(enable);
            process(1);
            if (!enable)
                delay_->hardReset();
        }

        /**
         * @brief Sets the sample rate for the delay. This ensures that time-based parameters remain accurate.
         *
         * @param sample_rate The new sample rate, in Hz.
         */
        virtual void setSampleRate(int sample_rate) override;

        /**
         * @brief Sets the oversample amount and updates internal delay buffer sizes accordingly.
         *
         * @param oversample The oversampling factor.
         */
        virtual void setOversampleAmount(int oversample) override;

        /**
         * @brief Processes audio input through the delay effect.
         *
         * This function applies the delay to the input signal using the current parameters (delay time, feedback, etc.)
         * and writes the processed audio to the output buffer.
         *
         * @param audio_in Pointer to the input audio samples.
         * @param num_samples The number of samples to process.
         */
        virtual void processWithInput(const poly_float* audio_in, int num_samples) override;

        /**
         * @brief Clones the DelayModule, creating a new instance with identical parameters.
         *
         * @return A pointer to the newly cloned DelayModule instance.
         */
        virtual Processor* clone() const override { return new DelayModule(*this); }

    protected:
        const Output* beats_per_second_; ///< An output providing tempo information for syncing delay times.
        StereoDelay* delay_;            ///< The underlying StereoDelay processor implementing the delay effect.

        JUCE_LEAK_DETECTOR(DelayModule)
    };
} // namespace vital
