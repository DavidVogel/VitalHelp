#include "sample_module.h"
#include "synth_constants.h"

namespace vital {

    /**
     * @brief Constructs the SampleModule, creating a new SampleSource and initializing state.
     */
    SampleModule::SampleModule() : SynthModule(kNumInputs, kNumOutputs), on_(nullptr) {
        sampler_ = new SampleSource();
        was_on_ = std::make_shared<bool>(true);
    }

    /**
     * @brief Initializes the SampleModule by creating and connecting controls to the SampleSource.
     *
     * Sets up various controls, including looping, bounce (reverse playback), keytracking, transposition,
     * tuning, level, and panning. The sampler is then connected to the module's outputs.
     */
    void SampleModule::init() {
        on_ = createBaseControl("sample_on");
        Value* random_phase = createBaseControl("sample_random_phase");
        Value* loop = createBaseControl("sample_loop");
        Value* bounce = createBaseControl("sample_bounce");
        Value* keytrack = createBaseControl("sample_keytrack");
        Value* transpose_quantize = createBaseControl("sample_transpose_quantize");
        Output* transpose = createPolyModControl("sample_transpose");
        Output* tune = createPolyModControl("sample_tune");
        Output* level = createPolyModControl("sample_level", true, true);
        Output* pan = createPolyModControl("sample_pan");

        // Connect inputs to the sampler.
        sampler_->useInput(input(kReset), SampleSource::kReset);
        sampler_->useInput(input(kMidi), SampleSource::kMidi);
        sampler_->useInput(input(kNoteCount), SampleSource::kNoteCount);

        // Connect control parameters to the sampler.
        sampler_->plug(random_phase, SampleSource::kRandomPhase);
        sampler_->plug(keytrack, SampleSource::kKeytrack);
        sampler_->plug(loop, SampleSource::kLoop);
        sampler_->plug(bounce, SampleSource::kBounce);
        sampler_->plug(transpose, SampleSource::kTranspose);
        sampler_->plug(transpose_quantize, SampleSource::kTransposeQuantize);
        sampler_->plug(tune, SampleSource::kTune);
        sampler_->plug(level, SampleSource::kLevel);
        sampler_->plug(pan, SampleSource::kPan);

        // Map sampler outputs.
        sampler_->useOutput(output(kRaw), SampleSource::kRaw);
        sampler_->useOutput(output(kLevelled), SampleSource::kLevelled);

        addProcessor(sampler_);
        SynthModule::init();
    }

    /**
     * @brief Processes audio. If the sample is on, it processes normally; if off, it clears outputs.
     *
     * @param num_samples The number of samples to process.
     */
    void SampleModule::process(int num_samples) {
        bool on = on_->value();

        if (on)
            SynthModule::process(num_samples);
        else if (*was_on_) {
            // Clear outputs if we're turning off the sample.
            output(kRaw)->clearBuffer();
            output(kLevelled)->clearBuffer();
            getPhaseOutput()->buffer[0] = 0.0f;
        }

        *was_on_ = on;
    }
} // namespace vital
