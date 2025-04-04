#pragma once

#include "synth_module.h"
#include "delay.h"

namespace vital {

    /**
     * @brief A chorus effect module that modulates delayed signals to create thickening and widening of the sound.
     *
     * The ChorusModule creates multiple delayed and modulated copies of the input audio signal. By mixing these delayed
     * signals back into the original, a richer and more complex sound is produced. The module can dynamically adjust:
     * - The number of voice pairs (sets of delayed signals),
     * - Delay times,
     * - Modulation depth and frequency,
     * - Wet/dry mix.
     */
    class ChorusModule : public SynthModule {
    public:
        /// The maximum modulation depth in seconds (for delay time modulation).
        static constexpr mono_float kMaxChorusModulation = 0.03f;
        /// The maximum chorus delay time in seconds.
        static constexpr mono_float kMaxChorusDelay = 0.08f;
        /// The maximum number of delay line pairs (voices).
        static constexpr int kMaxDelayPairs = 4;

        /**
         * @brief Constructs a ChorusModule.
         *
         * @param beats_per_second Output that provides tempo (beats per second) for tempo-synced operations.
         */
        ChorusModule(const Output* beats_per_second);
        virtual ~ChorusModule() { }

        /**
         * @brief Initializes the chorus module, setting up controls and linking parameters.
         */
        void init() override;

        /**
         * @brief Enables or disables the chorus module.
         *
         * When enabling, it resets some internal states, ensuring a clean start for the effect.
         *
         * @param enable True to enable, false to disable.
         */
        void enable(bool enable) override;

        /**
         * @brief Processes the input audio through the chorus effect.
         *
         * This method reads from `audio_in`, applies delay lines, modulation, and mixing, and writes the
         * processed signal to the output buffer.
         *
         * @param audio_in    Pointer to the input audio samples.
         * @param num_samples The number of samples to process.
         */
        void processWithInput(const poly_float* audio_in, int num_samples) override;

        /**
         * @brief Adjusts the internal phase to align with a given time, useful for syncing to host position.
         *
         * @param seconds The time in seconds to which the chorus should be aligned.
         */
        void correctToTime(double seconds) override;

        /**
         * @brief Clones this chorus module.
         *
         * @return A pointer to the cloned Processor.
         * @note This module currently does not support cloning (returns nullptr).
         */
        Processor* clone() const override { VITAL_ASSERT(false); return nullptr; }

        /**
         * @brief Retrieves and updates the number of active voice pairs based on control inputs.
         *
         * Also handles resetting delay lines if new voices are added.
         *
         * @return The updated number of voice pairs.
         */
        int getNextNumVoicePairs();

    protected:
        const Output* beats_per_second_; ///< A reference for tempo synchronization.
        Value* voices_;                 ///< Control for the number of chorus voices.

        int last_num_voices_;           ///< Tracks the last known number of voices to detect changes.

        cr::Output delay_status_outputs_[kMaxDelayPairs]; ///< Outputs for delay status or frequency debug information.

        Output* frequency_;   ///< Control for modulation frequency (can be free-running or tempo-synced).
        Output* delay_time_1_;///< Control for the first delay time parameter.
        Output* delay_time_2_;///< Control for the second delay time parameter.
        Output* mod_depth_;   ///< Control for modulation depth.
        Output* wet_output_;  ///< Control for the wet/dry mix.

        poly_float phase_;    ///< Current modulation phase.
        poly_float wet_;      ///< Current wet amount (for wet/dry mixing).
        poly_float dry_;      ///< Current dry amount (for wet/dry mixing).

        poly_float delay_input_buffer_[kMaxBufferSize]; ///< Temporary buffer for input before delays.

        cr::Value delay_frequencies_[kMaxDelayPairs]; ///< Holds frequency parameter values for each delay line.
        MultiDelay* delays_[kMaxDelayPairs]; ///< The delay processors that implement the chorus voices.

        JUCE_LEAK_DETECTOR(ChorusModule)
    };
} // namespace vital
