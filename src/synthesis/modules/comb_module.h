#pragma once

#include "synth_module.h"
#include "synth_constants.h"

namespace vital {

    class CombFilter;

    /**
     * @brief A module that implements a comb filter effect.
     *
     * The CombModule processes audio through a CombFilter, providing parameters
     * for cutoff, blending, resonance, and style. It interacts with MIDI-based
     * inputs to control the filter frequency and blending behaviors.
     */
    class CombModule : public SynthModule {
    public:
        /// Maximum number of feedback samples in the comb filter.
        static constexpr int kMaxFeedbackSamples = 25000;

        /**
         * @brief Input parameter indices for the CombModule.
         *
         * - kAudio:             The audio input signal to be filtered.
         * - kReset:             Resets the comb filter's internal state.
         * - kMidiCutoff:        A MIDI-based cutoff value that sets the comb filter's frequency.
         * - kMidiBlendTranspose: A parameter that mixes in a transposed version of the MIDI cutoff.
         * - kFilterCutoffBlend: A blending control to mix between different cutoff or filter modes.
         * - kStyle:             Determines the style or mode of the comb filter.
         * - kResonance:         Controls how resonant or pronounced the comb peaks are.
         * - kMidi:              MIDI-related input, possibly to track pitch or note events.
         */
        enum {
            kAudio,
            kReset,
            kMidiCutoff,
            kMidiBlendTranspose,
            kFilterCutoffBlend,
            kStyle,
            kResonance,
            kMidi,
            kNumInputs
        };

        /**
         * @brief Constructs a CombModule.
         *
         * Initializes a module that has the capability to apply a comb filter to its input audio signal.
         */
        CombModule();
        virtual ~CombModule() { }

        /**
         * @brief Initializes the CombModule, creating and connecting the internal CombFilter.
         */
        void init() override;

        /**
         * @brief Resets the comb filter with a given mask to handle polyphonic voices.
         *
         * @param reset_mask A mask indicating which voices should be reset.
         */
        void reset(poly_mask reset_mask) override;

        /**
         * @brief Performs a hard reset of the comb filter, returning it to its initial state.
         */
        void hardReset() override;

        /**
         * @brief Clones the CombModule, creating a new instance with the same configuration.
         *
         * @return A pointer to the newly cloned CombModule instance.
         */
        virtual Processor* clone() const override { return new CombModule(*this); }

    protected:
        CombFilter* comb_filter_; ///< The internal CombFilter processor used by this module.

        JUCE_LEAK_DETECTOR(CombModule)
    };
} // namespace vital
