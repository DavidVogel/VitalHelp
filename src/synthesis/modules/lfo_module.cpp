#include "lfo_module.h"

#include "line_generator.h"
#include "synth_lfo.h"

namespace vital {

    LfoModule::LfoModule(const std::string& prefix, LineGenerator* line_generator, const Output* beats_per_second) :
            SynthModule(kNumInputs, kNumOutputs),
            prefix_(prefix),
            beats_per_second_(beats_per_second) {
        // Create a SynthLfo processor and add it to this module's processing chain.
        lfo_ = new SynthLfo(line_generator);
        addProcessor(lfo_);

        // Default to control-rate processing.
        setControlRate(true);
    }

    void LfoModule::init() {
        /**
         * @brief Initializes LFO parameters, connecting them to the SynthLfo.
         *
         * It creates modulation controls for frequency, phase, fade time, delay time, stereo phase offset,
         * and smooth time. It also sets up a sync type control and links the LFO inputs/outputs to this module.
         */
        Output* free_frequency = createPolyModControl(prefix_ + "_frequency");
        Output* phase = createPolyModControl(prefix_ + "_phase");
        Output* fade = createPolyModControl(prefix_ + "_fade_time");
        Output* delay = createPolyModControl(prefix_ + "_delay_time");
        Output* stereo_phase = createPolyModControl(prefix_ + "_stereo");
        Value* sync_type = createBaseControl(prefix_ + "_sync_type");
        Value* smooth_mode = createBaseControl(prefix_ + "_smooth_mode");
        Output* smooth_time = createPolyModControl(prefix_ + "_smooth_time");

        // Create a tempo-sync switch for frequency if needed.
        // The 'true' at the end indicates this frequency depends on MIDI input.
        Output* frequency = createTempoSyncSwitch(prefix_, free_frequency->owner, beats_per_second_, true, input(kMidi));

        // Use the note trigger and note count inputs:
        lfo_->useInput(input(kNoteTrigger), SynthLfo::kNoteTrigger);
        lfo_->useInput(input(kNoteCount), SynthLfo::kNoteCount);

        // Link LFO outputs to this module's outputs:
        lfo_->useOutput(output(kValue), SynthLfo::kValue);
        lfo_->useOutput(output(kOscPhase), SynthLfo::kPhase);
        lfo_->useOutput(output(kOscFrequency), SynthLfo::kOscFrequency);

        // Plug parameters into the SynthLfo:
        lfo_->plug(frequency, SynthLfo::kFrequency);
        lfo_->plug(phase, SynthLfo::kPhase);
        lfo_->plug(stereo_phase, SynthLfo::kStereoPhase);
        lfo_->plug(sync_type, SynthLfo::kSyncType);
        lfo_->plug(smooth_mode, SynthLfo::kSmoothMode);
        lfo_->plug(fade, SynthLfo::kFade);
        lfo_->plug(smooth_time, SynthLfo::kSmoothTime);
        lfo_->plug(delay, SynthLfo::kDelay);
    }

    void LfoModule::correctToTime(double seconds) {
        /**
         * @brief Aligns the LFO's internal phase with a given time reference.
         */
        lfo_->correctToTime(seconds);
    }

    void LfoModule::setControlRate(bool control_rate) {
        /**
         * @brief Updates whether the LFO runs at control or audio-rate and notifies the SynthLfo.
         */
        Processor::setControlRate(control_rate);
        lfo_->setControlRate(control_rate);
    }
} // namespace vital
