#pragma once

#include "synth_module.h"
#include "formant_filter.h"

namespace vital {

    /**
     * @brief A module that applies various formant filtering styles to an incoming audio signal.
     *
     * The FormantModule provides a selection of different formant filter styles, allowing
     * morphing between vowel shapes or other spectral formant distributions. Parameters such as
     * formant position (X, Y), transpose, resonance, and spread can be controlled, and the filter
     * style can be changed dynamically.
     */
    class FormantModule : public SynthModule {
    public:
        /**
         * @brief Input parameter indices for the FormantModule.
         *
         * - kAudio:    The input audio signal to be filtered.
         * - kReset:    Reset trigger to reinitialize the filter state.
         * - kResonance: Resonance control for the formant filters.
         * - kBlend:    Blending control, used by the VocalTract style for interpolation.
         * - kStyle:    Selects which formant filter style to use.
         */
        enum {
            kAudio,
            kReset,
            kResonance,
            kBlend,
            kStyle,
            kNumInputs
        };

        /**
         * @brief Constructs a FormantModule, optionally with a parameter prefix for distinguishing multiple instances.
         *
         * @param prefix A string prefix applied to parameter names.
         */
        FormantModule(std::string prefix = "");
        virtual ~FormantModule() { }

        /**
         * @brief Creates a modulation control (parameter) that can be mono or poly depending on the module settings.
         *
         * @param name        The name of the parameter.
         * @param audio_rate  True for audio-rate modulation, false for control-rate.
         * @param smooth_value True if the parameter changes should be smoothed over time.
         * @return A pointer to the newly created Output parameter object.
         */
        Output* createModControl(std::string name, bool audio_rate = false, bool smooth_value = false);

        /**
         * @brief Initializes the FormantModule, creating formant filters and connecting parameters.
         */
        void init() override;

        /**
         * @brief Processes a block of samples. Chooses the appropriate formant filter style and processes audio.
         *
         * @param num_samples The number of samples to process in the current block.
         */
        void process(int num_samples) override;

        /**
         * @brief Resets the currently active formant filter for a given set of voices (indicated by the reset_mask).
         *
         * @param reset_mask A mask indicating which voices should be reset.
         */
        void reset(poly_mask reset_mask) override;

        /**
         * @brief Performs a hard reset of the currently active formant filter, clearing its internal state.
         */
        void hardReset() override;

        /**
         * @brief Sets whether the module is operating in mono mode or not.
         *
         * @param mono True if the module should be mono, false if polyphonic.
         */
        void setMono(bool mono) { mono_ = mono; }

        /**
         * @brief Creates a clone of this FormantModule with identical settings.
         *
         * @return A pointer to the newly cloned FormantModule instance.
         */
        virtual Processor* clone() const override { return new FormantModule(*this); }

    protected:
        /**
         * @brief Sets the currently active formant filter style, enabling and disabling submodules as necessary.
         *
         * @param new_style The index of the new style (from 0 to FormantFilter::kTotalFormantFilters - 1).
         */
        void setStyle(int new_style);

        std::string prefix_;                                 ///< Prefix for parameter naming.
        Processor* formant_filters_[FormantFilter::kTotalFormantFilters]; ///< Array holding all possible formant filter styles.
        int last_style_;                                     ///< Tracks the currently active style.
        bool mono_;                                          ///< True if running in mono mode, false for polyphonic.

        JUCE_LEAK_DETECTOR(FormantModule)
    };
} // namespace vital
