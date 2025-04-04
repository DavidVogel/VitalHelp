#include "flanger_module.h"
#include "delay.h"
#include "memory.h"
#include "synth_constants.h"

namespace vital {

    FlangerModule::FlangerModule(const Output* beats_per_second) :
            SynthModule(0, kNumOutputs),
            beats_per_second_(beats_per_second),
            frequency_(nullptr), phase_offset_(nullptr), mod_depth_(nullptr),
            phase_(0.0f), delay_(nullptr) { }

    FlangerModule::~FlangerModule() { }

    void FlangerModule::init() {
        /**
         * @brief Initializes the flanger, setting up a large enough delay buffer and linking parameters.
         *
         * It creates modulation controls for frequency, depth, center delay, feedback, wet/dry mix,
         * and a phase offset for stereo effects. The StereoDelay is configured to be unfiltered and clamped.
         */
        static constexpr int kMaxSamples = 40000;
        static const cr::Value kDelayStyle(StereoDelay::kClampedUnfiltered);

        delay_ = new StereoDelay(kMaxSamples);
        addIdleProcessor(delay_);
        phase_ = 0.0f;
        delay_->useOutput(output(kAudioOutput));

        Output* free_frequency = createMonoModControl("flanger_frequency");
        frequency_ = createTempoSyncSwitch("flanger", free_frequency->owner, beats_per_second_, false);
        center_ = createMonoModControl("flanger_center");
        Output* feedback = createMonoModControl("flanger_feedback");
        Output* wet = createMonoModControl("flanger_dry_wet");
        mod_depth_ = createMonoModControl("flanger_mod_depth");

        phase_offset_ = createMonoModControl("flanger_phase_offset");

        // Plugging parameters into the StereoDelay
        delay_->plug(&delay_frequency_, StereoDelay::kFrequency);
        delay_->plug(feedback, StereoDelay::kFeedback);
        delay_->plug(wet, StereoDelay::kWet);
        delay_->plug(&kDelayStyle, StereoDelay::kStyle);

        SynthModule::init();
    }

    void FlangerModule::processWithInput(const poly_float* audio_in, int num_samples) {
        /**
         * @brief Applies the flanger effect.
         *
         * Updates the modulation phase based on frequency and sample rate, calculates a delay time from center and depth,
         * and sets this delay time by adjusting the delay_frequency_ parameter. The StereoDelay then processes the input audio.
         */
        static constexpr float kMaxFrequency = 20000.0f;

        SynthModule::process(num_samples);
        poly_float frequency = frequency_->buffer[0];
        poly_float delta_phase = (frequency * num_samples) / getSampleRate();
        phase_ = utils::mod(phase_ + delta_phase);

        poly_float phase_offset = phase_offset_->buffer[0];
        poly_float right_offset = (phase_offset & constants::kRightMask);
        poly_float phase_total = phase_ - phase_offset / 2.0f + right_offset;

        // Compute modulation as a triangle waveform varying from -1 to 1, then map it with depth
        poly_float mod = mod_depth_->buffer[0] * (utils::triangleWave(phase_total) * 2.0f - 1.0f) + 1.0f;

        // Calculate the effective delay time from center (converted from MIDI note to frequency)
        poly_float delay = poly_float(1.0f) / utils::midiNoteToFrequency(center_->buffer[0]);
        delay = (delay - kModulationDelayBuffer) * mod + kModulationDelayBuffer;
        poly_float delay_frequency = poly_float(1.0f) / utils::max(delay, 1.0f / kMaxFrequency);

        output(kFrequencyOutput)->buffer[0] = delay_frequency;
        delay_frequency_.set(delay_frequency);
        delay_->processWithInput(audio_in, num_samples);
    }

    void FlangerModule::correctToTime(double seconds) {
        /**
         * @brief Aligns the flanger modulation phase to a given time, useful for sync to host timelines.
         */
        phase_ = utils::getCycleOffsetFromSeconds(seconds, frequency_->buffer[0]);
    }
} // namespace vital
