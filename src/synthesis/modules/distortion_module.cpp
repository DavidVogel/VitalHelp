#include "distortion_module.h"

#include "distortion.h"
#include "digital_svf.h"

namespace vital {

    DistortionModule::DistortionModule() :
            SynthModule(0, 1), // no inputs registered here, 1 output
            distortion_(nullptr),
            filter_(nullptr),
            mix_(0.0f) { }

    DistortionModule::~DistortionModule() {
        // Clean up owned processors if needed.
    }

    void DistortionModule::init() {
        /**
         * @brief Initializes the internal processors and control parameters for the distortion and filter.
         *
         * It creates a Distortion processor and links it to output. Then sets up the Drive and Mix controls,
         * as well as filter parameters such as cutoff and resonance. The filter can be inserted before or after
         * distortion depending on the filter_order_ parameter.
         */
        distortion_ = new Distortion();
        distortion_->useOutput(output());
        addIdleProcessor(distortion_);

        Value* distortion_type = createBaseControl("distortion_type");
        Output* distortion_drive = createMonoModControl("distortion_drive", true, true);
        distortion_mix_ = createMonoModControl("distortion_mix");

        distortion_->plug(distortion_type, Distortion::kType);
        distortion_->plug(distortion_drive, Distortion::kDrive);

        filter_order_ = createBaseControl("distortion_filter_order");
        Output* midi_cutoff = createMonoModControl("distortion_filter_cutoff", true, true);
        Output* resonance = createMonoModControl("distortion_filter_resonance");
        Output* blend = createMonoModControl("distortion_filter_blend");

        filter_ = new DigitalSvf();
        filter_->useOutput(output());
        filter_->plug(midi_cutoff, DigitalSvf::kMidiCutoff);
        filter_->plug(resonance, DigitalSvf::kResonance);
        filter_->plug(blend, DigitalSvf::kPassBlend);
        filter_->setDriveCompensation(false);
        filter_->setBasic(true);
        addIdleProcessor(filter_);

        SynthModule::init();
    }

    void DistortionModule::setSampleRate(int sample_rate) {
        /**
         * @brief Updates sample rate for both the Distortion and filter.
         *
         * This ensures that parameters dependent on sample rate, such as filter cutoff or
         * certain distortion algorithms, remain accurate at different playback rates.
         */
        SynthModule::setSampleRate(sample_rate);
        distortion_->setSampleRate(sample_rate);
        filter_->setSampleRate(sample_rate);
    }

    void DistortionModule::processWithInput(const poly_float* audio_in, int num_samples) {
        /**
         * @brief Processes a block of audio samples through the distortion and filter stages.
         *
         * The order of processing (distortion->filter, filter->distortion, or just distortion) is decided by filter_order_.
         * After processing, a final wet/dry mix is applied to blend the processed output with the original signal.
         */
        SynthModule::process(num_samples);

        // Determine processing order based on filter order value
        float order_value = filter_order_->output()->buffer[0][0];
        if (order_value < 1.0f) {
            // Only distortion
            distortion_->processWithInput(audio_in, num_samples);
        }
        else if (order_value > 1.0f) {
            // Distortion followed by filter
            distortion_->processWithInput(audio_in, num_samples);
            filter_->processWithInput(output()->buffer, num_samples);
        }
        else {
            // Filter first, then distortion
            filter_->processWithInput(audio_in, num_samples);
            distortion_->processWithInput(output()->buffer, num_samples);
        }

        poly_float current_mix = mix_;
        mix_ = utils::clamp(distortion_mix_->buffer[0], 0.0f, 1.0f);
        poly_float delta_mix = (mix_ - current_mix) * (1.0f / num_samples);
        poly_float* audio_out = output()->buffer;

        // Apply wet/dry interpolation over the block of samples.
        for (int i = 0; i < num_samples; ++i) {
            current_mix += delta_mix;
            // Interpolate between the original audio and the processed output
            audio_out[i] = utils::interpolate(audio_in[i], audio_out[i], current_mix);
        }
    }
} // namespace vital
