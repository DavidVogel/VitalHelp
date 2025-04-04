#pragma once

#include "synth_module.h"
#include "formant_module.h"

namespace vital {

    class CombModule;
    class DigitalSvf;
    class DiodeFilter;
    class DirtyFilter;
    class LadderFilter;
    class PhaserFilter;
    class SallenKeyFilter;

    /**
     * @brief A versatile filter module supporting multiple filter models and mixing options.
     *
     * The FilterModule provides a unified interface to various filter types (comb, digital SVF, diode, dirty,
     * formant, ladder, phaser, and sallen-key filters). It can switch between these models dynamically,
     * control parameters such as cutoff, resonance, drive, and filter style, and blend the filtered output
     * with the original signal. The module can be configured to operate in mono or polyphonic mode and
     * can track MIDI input for keytracking the cutoff frequency.
     */
    class FilterModule : public SynthModule {
    public:
        /**
         * @brief Input parameter indices for the FilterModule.
         *
         * - kAudio:   The input audio signal to be filtered.
         * - kReset:   A reset trigger input.
         * - kKeytrack: A keytrack input for adjusting filter cutoff based on note pitch.
         * - kMidi:     MIDI-related input for advanced control (e.g., from MIDI notes).
         */
        enum {
            kAudio,
            kReset,
            kKeytrack,
            kMidi,
            kNumInputs
        };

        /**
         * @brief Constructs a FilterModule, optionally with a prefix for parameter naming.
         *
         * @param prefix A string used as a prefix for the filter parameters, allowing multiple filter instances to coexist.
         */
        FilterModule(std::string prefix = "");
        virtual ~FilterModule() { }

        /**
         * @brief Processes a block of samples, applying the selected filter model and mixing the result.
         *
         * The module checks if it's turned on, selects the correct filter model, processes the input audio,
         * and then applies the wet/dry mix.
         *
         * @param num_samples The number of samples to process.
         */
        void process(int num_samples) override;

        /**
         * @brief Sets whether a "on/off" value control should be created.
         *
         * @param create_on_value If true, creates a control for turning the filter on or off.
         */
        void setCreateOnValue(bool create_on_value) { create_on_value_ = create_on_value; }

        /**
         * @brief Configures the FilterModule to be mono or polyphonic.
         *
         * @param mono True for mono operation, false for polyphonic operation.
         */
        void setMono(bool mono);

        /**
         * @brief Creates a modulation control parameter for the filter.
         *
         * This utility method handles whether the parameter is mono or poly, and if it should be smooth or audio-rate.
         *
         * @param name               The base name of the parameter.
         * @param audio_rate         True if the parameter should be audio-rate modulated.
         * @param smooth_value       True if the parameter changes should be smoothed.
         * @param internal_modulation An optional internal modulation source.
         * @return A pointer to the created Output object representing this control.
         */
        Output* createModControl(std::string name, bool audio_rate = false, bool smooth_value = false,
                                 Output* internal_modulation = nullptr);

        /**
         * @brief Initializes the filter module, creating parameters and linking them to submodules.
         */
        void init() override;

        /**
         * @brief Performs a hard reset of all filter submodules, clearing their states and buffers.
         */
        void hardReset() override;

        /**
         * @brief Clones the filter module, creating a new instance with identical settings.
         *
         * @return A pointer to the newly cloned FilterModule instance.
         */
        virtual Processor* clone() const override {
            FilterModule* newModule = new FilterModule(*this);
            newModule->last_model_ = -1;
            return newModule;
        }

        /**
         * @brief Retrieves the "on" Value for controlling the filter state if it was created.
         *
         * @return A pointer to the Value representing the on/off state of the filter.
         */
        const Value* getOnValue() { return on_; }

    protected:
        /**
         * @brief Sets the current filter model, enabling the corresponding submodule and disabling others.
         *
         * This method handles switching between filter types (comb, digital SVF, diode, etc.) and
         * resets the newly selected filter to ensure a clean start.
         *
         * @param new_model An integer representing the filter model (defined in synth_constants.h).
         */
        void setModel(int new_model);

        int last_model_;         ///< Tracks the last active filter model to detect changes.
        bool was_on_;            ///< Tracks whether the filter was previously on or off.
        std::string prefix_;     ///< A prefix for naming parameters, enabling multiple instances.
        bool create_on_value_;   ///< Whether an "on" parameter should be created.
        bool mono_;              ///< True if the filter is in mono mode, false for polyphonic.

        Value* on_;              ///< Optional "on" control parameter.
        Value* filter_model_;    ///< A parameter selecting which filter model to use.
        poly_float mix_;         ///< Current wet/dry mix value for blending filtered audio with the original.

        Output* filter_mix_;     ///< The parameter output controlling the wet/dry mix.

        CombModule* comb_filter_;       ///< Comb filter submodule.
        DigitalSvf* digital_svf_;       ///< Digital state-variable filter submodule.
        DiodeFilter* diode_filter_;     ///< Diode filter submodule.
        DirtyFilter* dirty_filter_;     ///< "Dirty" filter submodule (non-linear, characterful).
        FormantModule* formant_filter_; ///< Formant filter submodule (for vowel/formant filtering).
        LadderFilter* ladder_filter_;   ///< Ladder filter submodule (classic analog-style filter).
        PhaserFilter* phaser_filter_;   ///< Phaser filter submodule (phase-based filtering).
        SallenKeyFilter* sallen_key_filter_; ///< Sallen-Key filter submodule (analog-inspired).

        JUCE_LEAK_DETECTOR(FilterModule)
    };
} // namespace vital
