#include "delay_module.h"

#include "delay.h"
#include "memory.h"

namespace vital {

    DelayModule::DelayModule(const Output* beats_per_second) : SynthModule(0, 1), beats_per_second_(beats_per_second) {
        int size = kMaxDelayTime * getSampleRate();
        // Initialize delay with max possible delay samples.
        delay_ = new StereoDelay(size);
        addIdleProcessor(delay_);
    }

    DelayModule::~DelayModule() { }

    void DelayModule::init() {
        /**
         * @brief Initializes the delay module by connecting the StereoDelay outputs and setting up parameter controls.
         *
         * This creates modulation controls for the delay frequency, feedback, wet/dry, and filtering parameters, as well as
         * a "style" parameter. The delay times can be free-running or tempo-synced using the provided beats_per_second_ reference.
         */
        delay_->useOutput(output());

        // Create mod controls and tempo sync switches for delay parameters.
        Output* free_frequency = createMonoModControl("delay_frequency");
        Output* frequency = createTempoSyncSwitch("delay", free_frequency->owner, beats_per_second_, false);
        Output* free_frequency_aux = createMonoModControl("delay_aux_frequency");
        Output* frequency_aux = createTempoSyncSwitch("delay_aux", free_frequency_aux->owner, beats_per_second_, false);
        Output* feedback = createMonoModControl("delay_feedback");
        Output* wet = createMonoModControl("delay_dry_wet");

        Output* filter_cutoff = createMonoModControl("delay_filter_cutoff");
        Output* filter_spread = createMonoModControl("delay_filter_spread");

        Value* style = createBaseControl("delay_style");

        // Plug parameters into the StereoDelay processor.
        delay_->plug(frequency, StereoDelay::kFrequency);
        delay_->plug(frequency_aux, StereoDelay::kFrequencyAux);
        delay_->plug(feedback, StereoDelay::kFeedback);
        delay_->plug(wet, StereoDelay::kWet);
        delay_->plug(style, StereoDelay::kStyle);
        delay_->plug(filter_cutoff, StereoDelay::kFilterCutoff);
        delay_->plug(filter_spread, StereoDelay::kFilterSpread);

        SynthModule::init();
    }

    void DelayModule::setSampleRate(int sample_rate) {
        /**
         * @brief Updates the delay processor's sample rate and maximum delay size.
         */
        SynthModule::setSampleRate(sample_rate);
        delay_->setSampleRate(sample_rate);
        delay_->setMaxSamples(kMaxDelayTime * getSampleRate());
    }

    void DelayModule::setOversampleAmount(int oversample) {
        /**
         * @brief Updates oversampling and recalculates maximum delay size based on the new effective sample rate.
         */
        SynthModule::setOversampleAmount(oversample);
        delay_->setMaxSamples(kMaxDelayTime * getSampleRate());
    }

    void DelayModule::processWithInput(const poly_float* audio_in, int num_samples) {
        /**
         * @brief Processes the input audio through the delay effect.
         *
         * First updates all internal parameters by calling SynthModule::process(),
         * then passes the samples to the StereoDelay for the actual delay processing.
         */
        SynthModule::process(num_samples);
        delay_->processWithInput(audio_in, num_samples);
    }

} // namespace vital
