#include "filter_module.h"

#include "comb_module.h"
#include "digital_svf.h"
#include "diode_filter.h"
#include "dirty_filter.h"
#include "formant_module.h"
#include "ladder_filter.h"
#include "phaser_filter.h"
#include "sallen_key_filter.h"
#include "synth_constants.h"

namespace vital {

    FilterModule::FilterModule(std::string prefix) :
            SynthModule(kNumInputs, 1),
            last_model_(-1),
            was_on_(false),
            prefix_(std::move(prefix)),
            create_on_value_(true),
            mono_(false),
            on_(nullptr),
            filter_model_(nullptr),
            mix_(0.0f),
            filter_mix_(nullptr),
            comb_filter_(nullptr),
            digital_svf_(nullptr),
            dirty_filter_(nullptr),
            formant_filter_(nullptr),
            ladder_filter_(nullptr),
            phaser_filter_(nullptr),
            sallen_key_filter_(nullptr) {

        // Create submodules for each filter type.
        comb_filter_ = new CombModule();
        digital_svf_ = new DigitalSvf();
        diode_filter_ = new DiodeFilter();
        dirty_filter_ = new DirtyFilter();
        ladder_filter_ = new LadderFilter();
        phaser_filter_ = new PhaserFilter(false);
        sallen_key_filter_ = new SallenKeyFilter();
        formant_filter_ = new FormantModule(prefix_);

        // Register submodules so they can be managed by the framework.
        addSubmodule(comb_filter_);
        addSubmodule(formant_filter_);

        addProcessor(comb_filter_);
        addProcessor(digital_svf_);
        addProcessor(diode_filter_);
        addProcessor(dirty_filter_);
        addProcessor(formant_filter_);
        addProcessor(ladder_filter_);
        addProcessor(phaser_filter_);
        addProcessor(sallen_key_filter_);
    }

    void FilterModule::init() {
        /**
         * @brief Initializes the filter parameters, creating controls for cutoff, resonance, drive, style, and mixing.
         *
         * It sets up keytracking, and for each filter submodule, connects the appropriate parameters.
         * The filter model can be switched at runtime, and only one model is enabled at a time.
         */
        Output* keytrack_amount = createModControl(prefix_ + "_keytrack");
        cr::Multiply* current_keytrack = new cr::Multiply();
        current_keytrack->useInput(input(kKeytrack), 0);
        current_keytrack->plug(keytrack_amount, 1);

        Output* midi_cutoff = createModControl(prefix_ + "_cutoff", true, true, current_keytrack->output());
        Output* resonance = createModControl(prefix_ + "_resonance");
        Output* drive = createModControl(prefix_ + "_drive");
        Output* blend = createModControl(prefix_ + "_blend");
        Output* blend_transpose = createModControl(prefix_ + "_blend_transpose");
        if (create_on_value_)
            on_ = createBaseControl(prefix_ + "_on");
        Value* filter_style = createBaseControl(prefix_ + "_style");
        filter_model_ = createBaseControl(prefix_ + "_model");

        filter_mix_ = createModControl(prefix_ + "_mix");

        // Wire up the comb filter:
        comb_filter_->useInput(input(kAudio), CombModule::kAudio);
        comb_filter_->plug(filter_style, CombModule::kStyle);
        comb_filter_->useInput(input(kReset), CombModule::kReset);
        comb_filter_->useInput(input(kMidi), CombModule::kMidi);
        comb_filter_->plug(midi_cutoff, CombModule::kMidiCutoff);
        comb_filter_->plug(blend_transpose, CombModule::kMidiBlendTranspose);
        comb_filter_->plug(blend, CombModule::kFilterCutoffBlend);
        comb_filter_->plug(resonance, CombModule::kResonance);
        comb_filter_->useOutput(output());

        // Digital SVF filter:
        digital_svf_->useInput(input(kAudio), DigitalSvf::kAudio);
        digital_svf_->plug(filter_style, DigitalSvf::kStyle);
        digital_svf_->plug(blend, DigitalSvf::kPassBlend);
        digital_svf_->useInput(input(kReset), DigitalSvf::kReset);
        digital_svf_->plug(midi_cutoff, DigitalSvf::kMidiCutoff);
        digital_svf_->plug(resonance, DigitalSvf::kResonance);
        digital_svf_->plug(drive, DigitalSvf::kDriveGain);
        digital_svf_->useOutput(output());

        // Diode filter:
        diode_filter_->useInput(input(kAudio), DiodeFilter::kAudio);
        diode_filter_->useInput(input(kReset), DiodeFilter::kReset);
        diode_filter_->plug(resonance, DiodeFilter::kResonance);
        diode_filter_->plug(filter_style, DiodeFilter::kStyle);
        diode_filter_->plug(blend, DiodeFilter::kPassBlend);
        diode_filter_->plug(midi_cutoff, DiodeFilter::kMidiCutoff);
        diode_filter_->plug(drive, DiodeFilter::kDriveGain);
        diode_filter_->useOutput(output());

        // Dirty filter:
        dirty_filter_->useInput(input(kAudio), DirtyFilter::kAudio);
        dirty_filter_->useInput(input(kReset), DirtyFilter::kReset);
        dirty_filter_->plug(resonance, DirtyFilter::kResonance);
        dirty_filter_->plug(filter_style, DirtyFilter::kStyle);
        dirty_filter_->plug(blend, DirtyFilter::kPassBlend);
        dirty_filter_->plug(midi_cutoff, DirtyFilter::kMidiCutoff);
        dirty_filter_->plug(drive, DirtyFilter::kDriveGain);
        dirty_filter_->useOutput(output());

        // Formant filter:
        formant_filter_->useInput(input(kAudio), FormantModule::kAudio);
        formant_filter_->useInput(input(kReset), FormantModule::kReset);
        formant_filter_->plug(blend, FormantModule::kBlend);
        formant_filter_->plug(filter_style, FormantModule::kStyle);
        formant_filter_->useOutput(output());

        // Ladder filter:
        ladder_filter_->useInput(input(kAudio), LadderFilter::kAudio);
        ladder_filter_->useInput(input(kReset), LadderFilter::kReset);
        ladder_filter_->plug(resonance, LadderFilter::kResonance);
        ladder_filter_->plug(filter_style, LadderFilter::kStyle);
        ladder_filter_->plug(blend, LadderFilter::kPassBlend);
        ladder_filter_->plug(midi_cutoff, LadderFilter::kMidiCutoff);
        ladder_filter_->plug(drive, LadderFilter::kDriveGain);
        ladder_filter_->useOutput(output());

        // Phaser filter:
        phaser_filter_->useInput(input(kAudio), PhaserFilter::kAudio);
        phaser_filter_->useInput(input(kReset), PhaserFilter::kReset);
        phaser_filter_->plug(resonance, PhaserFilter::kResonance);
        phaser_filter_->plug(filter_style, PhaserFilter::kStyle);
        phaser_filter_->plug(blend_transpose, PhaserFilter::kTranspose);
        phaser_filter_->plug(blend, PhaserFilter::kPassBlend);
        phaser_filter_->plug(midi_cutoff, PhaserFilter::kMidiCutoff);
        phaser_filter_->plug(drive, PhaserFilter::kDriveGain);
        phaser_filter_->useOutput(output());

        // Sallen-Key filter:
        sallen_key_filter_->plug(filter_style, SallenKeyFilter::kStyle);
        sallen_key_filter_->useInput(input(kAudio), SallenKeyFilter::kAudio);
        sallen_key_filter_->plug(blend, SallenKeyFilter::kPassBlend);
        sallen_key_filter_->useInput(input(kReset), SallenKeyFilter::kReset);
        sallen_key_filter_->plug(midi_cutoff, SallenKeyFilter::kMidiCutoff);
        sallen_key_filter_->plug(resonance, SallenKeyFilter::kResonance);
        sallen_key_filter_->plug(drive, SallenKeyFilter::kDriveGain);
        sallen_key_filter_->useOutput(output());

        // Disable all but dynamically chosen filter models initially:
        comb_filter_->enable(false);
        digital_svf_->enable(false);
        diode_filter_->enable(false);
        dirty_filter_->enable(false);
        formant_filter_->enable(false);
        ladder_filter_->enable(false);
        sallen_key_filter_->enable(false);

        addProcessor(current_keytrack);

        SynthModule::init();
    }

    void FilterModule::hardReset() {
        /**
         * @brief Resets all submodules to ensure no residual states.
         */
        comb_filter_->hardReset();
        digital_svf_->hardReset();
        diode_filter_->hardReset();
        dirty_filter_->hardReset();
        formant_filter_->hardReset();
        ladder_filter_->hardReset();
        phaser_filter_->hardReset();
        sallen_key_filter_->hardReset();
    }

    Output* FilterModule::createModControl(std::string name, bool audio_rate, bool smooth_value,
                                           Output* internal_modulation) {
        /**
         * @brief Creates a modulation control parameter, choosing between mono or poly variants.
         */
        if (mono_)
            return createMonoModControl(name, audio_rate, smooth_value, internal_modulation);
        return createPolyModControl(name, audio_rate, smooth_value, internal_modulation, input(kReset));
    }

    force_inline void FilterModule::setModel(int new_model) {
        /**
         * @brief Enables the new filter model and disables others, then resets if the model changed.
         */
        comb_filter_->enable(new_model == constants::kComb);
        digital_svf_->enable(new_model == constants::kDigital);
        diode_filter_->enable(new_model == constants::kDiode);
        dirty_filter_->enable(new_model == constants::kDirty);
        formant_filter_->enable(new_model == constants::kFormant);
        ladder_filter_->enable(new_model == constants::kLadder);
        phaser_filter_->enable(new_model == constants::kPhase);
        sallen_key_filter_->enable(new_model == constants::kAnalog);

        if (new_model == last_model_)
            return;

        Processor* to_reset = nullptr;
        if (new_model == constants::kAnalog) to_reset = sallen_key_filter_;
        else if (new_model == constants::kComb) to_reset = comb_filter_;
        else if (new_model == constants::kDigital) to_reset = digital_svf_;
        else if (new_model == constants::kDiode) to_reset = diode_filter_;
        else if (new_model == constants::kDirty) to_reset = dirty_filter_;
        else if (new_model == constants::kFormant) to_reset = formant_filter_;
        else if (new_model == constants::kLadder) to_reset = ladder_filter_;
        else if (new_model == constants::kPhase) to_reset = phaser_filter_;

        if (to_reset)
            getLocalProcessor(to_reset)->hardReset();

        last_model_ = new_model;
    }

    void FilterModule::process(int num_samples) {
        /**
         * @brief Processes audio through the selected filter model if the filter is on, and applies mixing.
         */
        bool on = on_ == nullptr || on_->value() > 0.5f;
        setModel(static_cast<int>(roundf(filter_model_->value())));

        if (on) {
            SynthModule::process(num_samples);

            poly_float current_mix = mix_;
            mix_ = utils::clamp(filter_mix_->buffer[0], 0.0f, 1.0f);
            current_mix = utils::maskLoad(current_mix, mix_, getResetMask(kReset));
            poly_float delta_mix = (mix_ - current_mix) * (1.0f / num_samples);

            poly_float* audio_out = output()->buffer;
            const poly_float* audio_in = input(kAudio)->source->buffer;
            for (int i = 0; i < num_samples; ++i) {
                current_mix += delta_mix;
                // Interpolate between dry (audio_in) and wet (audio_out) based on current_mix
                audio_out[i] = utils::interpolate(audio_in[i], audio_out[i], current_mix);
            }
        }
        else {
            // Filter is off: output silence
            utils::zeroBuffer(output()->buffer, num_samples);
        }
    }

    void FilterModule::setMono(bool mono) {
        /**
         * @brief Sets the filter to mono mode and updates the formant submodule accordingly.
         */
        mono_ = mono;
        formant_filter_->setMono(mono);
    }

} // namespace vital
