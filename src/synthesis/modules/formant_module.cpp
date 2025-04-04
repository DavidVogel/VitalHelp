#include "formant_module.h"
#include "vocal_tract.h"

namespace vital {

    FormantModule::FormantModule(std::string prefix) :
            SynthModule(kNumInputs, 1),
            prefix_(std::move(prefix)),
            formant_filters_(),
            last_style_(0),
            mono_(false) { }

    Output* FormantModule::createModControl(std::string name, bool audio_rate, bool smooth_value) {
        /**
         * @brief Creates a modulation parameter either as mono or poly depending on the mono_ flag.
         *
         * If mono_, creates a mono mod control; otherwise, creates a poly mod control,
         * connecting the poly mod control's reset input to this module's reset input for proper voice handling.
         */
        if (mono_)
            return createMonoModControl(name, audio_rate, smooth_value);
        return createPolyModControl(name, audio_rate, smooth_value, nullptr, input(kReset));
    }

    void FormantModule::init() {
        /**
         * @brief Initializes all formant filter styles and a VocalTract processor, plugging in parameters for X, Y, transpose, etc.
         */
        Output* formant_x = createModControl(prefix_ + "_formant_x", true, true);
        Output* formant_y = createModControl(prefix_ + "_formant_y", true, true);
        Output* formant_transpose = createModControl(prefix_ + "_formant_transpose", true, true);
        Output* formant_resonance = createModControl(prefix_ + "_formant_resonance");
        Output* formant_spread = createModControl(prefix_ + "_formant_spread");

        // Create all FormantFilter styles
        for (int i = 0; i < FormantFilter::kNumFormantStyles; ++i) {
            FormantFilter* formant_filter = new FormantFilter(i);
            formant_filters_[i] = formant_filter;
            addProcessor(formant_filter);
            formant_filter->enable(false);

            formant_filter->useInput(input(kAudio), FormantFilter::kAudio);
            formant_filter->useInput(input(kReset), FormantFilter::kReset);
            formant_filter->plug(formant_spread, FormantFilter::kSpread);
            formant_filter->plug(formant_x, FormantFilter::kInterpolateX);
            formant_filter->plug(formant_y, FormantFilter::kInterpolateY);
            formant_filter->plug(formant_transpose, FormantFilter::kTranspose);
            formant_filter->plug(formant_resonance, FormantFilter::kResonance);
            formant_filter->useOutput(output());
        }

        // Create VocalTract style and integrate it as one of the formant_filters_
        VocalTract* vocal_tract = new VocalTract();
        vocal_tract->useInput(input(kAudio), VocalTract::kAudio);
        vocal_tract->useInput(input(kReset), VocalTract::kReset);
        vocal_tract->useInput(input(kBlend), VocalTract::kBlend);
        vocal_tract->plug(formant_x, VocalTract::kTonguePosition);
        vocal_tract->plug(formant_y, VocalTract::kTongueHeight);
        vocal_tract->useOutput(output());
        formant_filters_[FormantFilter::kVocalTract] = vocal_tract;
        addProcessor(vocal_tract);
        vocal_tract->enable(false);

        // Enable the default style
        formant_filters_[last_style_]->enable(true);

        SynthModule::init();
    }

    void FormantModule::process(int num_samples) {
        /**
         * @brief Reads the style parameter from input(kStyle) and sets the active formant filter style accordingly.
         * Then processes the current block of audio samples through the active formant filter.
         */
        mono_float max_style = FormantFilter::kTotalFormantFilters - 1;
        int style = static_cast<int>(utils::clamp(input(kStyle)->at(0)[0], 0.0f, max_style));
        setStyle(style);

        SynthModule::process(num_samples);
    }

    void FormantModule::reset(poly_mask reset_mask) {
        /**
         * @brief Resets the currently active formant filter.
         */
        getLocalProcessor(formant_filters_[last_style_])->reset(reset_mask);
    }

    void FormantModule::hardReset() {
        /**
         * @brief Performs a hard reset on the currently active formant filter.
         */
        getLocalProcessor(formant_filters_[last_style_])->hardReset();
    }

    force_inline void FormantModule::setStyle(int new_style) {
        /**
         * @brief Switches the active formant filter style, enabling the new one and disabling the old one.
         * Also resets the newly activated filter to ensure a clean start.
         */
        if (last_style_ == new_style)
            return;

        formant_filters_[last_style_]->enable(false);
        formant_filters_[new_style]->enable(true);
        last_style_ = new_style;
        reset(constants::kFullMask);
    }
} // namespace vital
