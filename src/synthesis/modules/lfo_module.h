#pragma once

#include "synth_module.h"

class LineGenerator;

namespace vital {

    class SynthLfo;

    /**
     * @brief A module that generates a Low-Frequency Oscillation (LFO) signal from a LineGenerator source.
     *
     * The LfoModule uses a SynthLfo processor, controlling parameters such as frequency, phase, fade time, delay time,
     * and synchronization type. It can run at control-rate or audio-rate and supports stereo offset and smoothing.
     * The output includes the LFO's value, phase, and frequency for use in modulating other parameters.
     */
    class LfoModule : public SynthModule {
    public:
        /**
         * @brief Input parameter indices for the LfoModule.
         *
         * - kNoteTrigger: Trigger input, used to start or retrigger the LFO.
         * - kNoteCount:   Number of active notes, can affect certain LFO behaviors or voice counts.
         * - kMidi:        MIDI-related input, used for synchronization or advanced modulation (e.g., note on/off).
         */
        enum {
            kNoteTrigger,
            kNoteCount,
            kMidi,
            kNumInputs
        };

        /**
         * @brief Output parameter indices for the LfoModule.
         *
         * - kValue:        The current LFO amplitude at each sample.
         * - kOscPhase:     The current LFO phase and voice information.
         * - kOscFrequency: The current LFO frequency in Hz.
         */
        enum {
            kValue,
            kOscPhase,
            kOscFrequency,
            kNumOutputs
        };

        /**
         * @brief Constructs an LfoModule.
         *
         * @param prefix A string prefix for naming parameters, allowing multiple LFOs in a patch.
         * @param line_generator A LineGenerator object providing data for complex LFO shapes.
         * @param beats_per_second Output pointer that provides tempo info for tempo-synced LFO rates.
         */
        LfoModule(const std::string& prefix, LineGenerator* line_generator, const Output* beats_per_second);
        virtual ~LfoModule() { }

        /**
         * @brief Initializes the LfoModule, creating parameters and linking them to the SynthLfo processor.
         */
        void init() override;

        /**
         * @brief Creates a clone of the LfoModule with identical settings.
         * @return A pointer to the newly cloned LfoModule instance.
         */
        virtual Processor* clone() const override { return new LfoModule(*this); }

        /**
         * @brief Aligns the LFO's phase to a specific time, useful for syncing to a host timeline.
         *
         * @param seconds The time in seconds to align the LFO to.
         */
        void correctToTime(double seconds) override;

        /**
         * @brief Sets whether the LFO should run at control-rate or audio-rate.
         *
         * @param control_rate True for control-rate processing, false for audio-rate.
         */
        void setControlRate(bool control_rate) override;

    protected:
        std::string prefix_;           ///< Prefix for parameter naming.
        SynthLfo* lfo_;                ///< The internal SynthLfo processor generating the LFO signal.
        const Output* beats_per_second_; ///< Reference for tempo synchronization.

        JUCE_LEAK_DETECTOR(LfoModule)
    };
} // namespace vital
