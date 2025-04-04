#include "comb_module.h"
#include "comb_filter.h"

namespace vital {

    CombModule::CombModule() : SynthModule(kNumInputs, 1), comb_filter_(nullptr) { }

    void CombModule::init() {
        /**
         * @brief Initializes the CombFilter and connects the input and output parameters.
         *
         * The comb filter receives various inputs (audio, MIDI cutoff, style, etc.) and outputs
         * the filtered signal. The SynthModule::init() finalizes initialization steps.
         */
        comb_filter_ = new CombFilter(kMaxFeedbackSamples);
        addProcessor(comb_filter_);

        // Connect inputs to the comb filter:
        comb_filter_->useInput(input(kAudio), CombFilter::kAudio);
        comb_filter_->useInput(input(kMidiCutoff), CombFilter::kMidiCutoff);
        comb_filter_->useInput(input(kStyle), CombFilter::kStyle);
        comb_filter_->useInput(input(kMidiBlendTranspose), CombFilter::kTranspose);
        comb_filter_->useInput(input(kFilterCutoffBlend), CombFilter::kPassBlend);
        comb_filter_->useInput(input(kResonance), CombFilter::kResonance);
        comb_filter_->useInput(input(kReset), CombFilter::kReset);

        // Connect the comb filter's output to the module's output:
        comb_filter_->useOutput(output());

        SynthModule::init();
    }

    void CombModule::reset(poly_mask reset_mask) {
        /**
         * @brief Resets specific voices of the comb filter based on a mask.
         *
         * This allows polyphonic resets so that individual voices can be started fresh
         * without disrupting all voices at once.
         */
        getLocalProcessor(comb_filter_)->reset(reset_mask);
    }

    void CombModule::hardReset() {
        /**
         * @brief Performs a hard reset on the comb filter,
         * clearing any buffered samples and resetting all states.
         */
        getLocalProcessor(comb_filter_)->hardReset();
    }
} // namespace vital
