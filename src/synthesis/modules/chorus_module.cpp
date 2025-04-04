#include "chorus_module.h"

#include "delay.h"
#include "memory.h"
#include "synth_constants.h"

namespace vital {

    ChorusModule::ChorusModule(const Output* beats_per_second) :
            SynthModule(0, 1),
            beats_per_second_(beats_per_second),
            frequency_(nullptr),
            delay_time_1_(nullptr),
            delay_time_2_(nullptr),
            mod_depth_(nullptr),
            phase_(0.0f) {
        wet_ = 0.0f;
        dry_ = 0.0f;
        last_num_voices_ = 0;
        int max_samples = kMaxChorusDelay * kMaxSampleRate + 1;

        // Allocate delay lines and register their outputs for debugging or analysis.
        for (int i = 0; i < kMaxDelayPairs; ++i) {
            registerOutput(&delay_status_outputs_[i]);
            delays_[i] = new MultiDelay(max_samples);
            addIdleProcessor(delays_[i]);
        }
    }

    void ChorusModule::init() {
        static const cr::Value kDelayStyle(MultiDelay::kMono);

        voices_ = createBaseControl("chorus_voices");

        // Create mod controls and link them with line generator or value sources.
        Output* free_frequency = createMonoModControl("chorus_frequency");
        frequency_ = createTempoSyncSwitch("chorus", free_frequency->owner, beats_per_second_, false);
        Output* feedback = createMonoModControl("chorus_feedback");
        wet_output_ = createMonoModControl("chorus_dry_wet");
        Output* cutoff = createMonoModControl("chorus_cutoff");
        Output* spread = createMonoModControl("chorus_spread");
        mod_depth_ = createMonoModControl("chorus_mod_depth");

        delay_time_1_ = createMonoModControl("chorus_delay_1");
        delay_time_2_ = createMonoModControl("chorus_delay_2");

        // Plug parameters into each delay line.
        for (int i = 0; i < kMaxDelayPairs; ++i) {
            delays_[i]->plug(&delay_frequencies_[i], MultiDelay::kFrequency);
            delays_[i]->plug(feedback, MultiDelay::kFeedback);
            delays_[i]->plug(&constants::kValueOne, MultiDelay::kWet);
            delays_[i]->plug(cutoff, MultiDelay::kFilterCutoff);
            delays_[i]->plug(spread, MultiDelay::kFilterSpread);
            delays_[i]->plug(&kDelayStyle, MultiDelay::kStyle);
        }

        SynthModule::init();
    }

    void ChorusModule::enable(bool enable) {
        SynthModule::enable(enable);
        process(1); // Ensure one block of processing to initialize states.
        if (enable) {
            // Reset wet/dry mix and delay lines on enable.
            wet_ = 0.0f;
            dry_ = 0.0f;

            for (int i = 0; i < kMaxDelayPairs; ++i)
                delays_[i]->hardReset();
        }
    }

    int ChorusModule::getNextNumVoicePairs() {
        int num_voice_pairs = voices_->value();

        // If number of voices increased, reset the newly added voices.
        for (int i = last_num_voices_; i < num_voice_pairs; ++i)
            delays_[i]->reset(constants::kFullMask);

        last_num_voices_ = num_voice_pairs;
        return num_voice_pairs;
    }

    void ChorusModule::processWithInput(const poly_float* audio_in, int num_samples) {
        // Update any internal parameters
        SynthModule::process(num_samples);

        // Calculate the modulation phase increment based on frequency.
        poly_float frequency = frequency_->buffer[0];
        poly_float delta_phase = (frequency * num_samples) * (1.0f / getSampleRate());
        phase_ = utils::mod(phase_ + delta_phase);

        poly_float* audio_out = output()->buffer;

        // Start by copying the input audio to output, forming the base (dry) signal.
        for (int s = 0; s < num_samples; ++s) {
            poly_float sample = audio_in[s] & constants::kFirstMask;
            // SwapVoices used to handle stereo processing where left/right are interleaved in poly_float.
            audio_out[s] = sample + utils::swapVoices(sample);
        }

        // Get the number of voices and prepare delay parameters.
        int num_voices = getNextNumVoicePairs();

        poly_float delay1 = delay_time_1_->buffer[0];
        poly_float delay2 = delay_time_2_->buffer[0];
        // Uses the first mask to select which delay time to use for left channel, interpolating for other channels.
        poly_float delay_time = utils::maskLoad(delay2, delay1, constants::kFirstMask);
        poly_float average_delay = (delay_time + utils::swapVoices(delay_time)) * 0.5f;

        // Apply modulation and set up delays:
        for (int i = 0; i < num_voices; ++i) {
            // Stagger phases for different voices to create a richer chorus texture.
            float pair_offset = i * 0.25f / num_voices;
            poly_float right_offset = (poly_float(0.25f) & constants::kRightMask);
            poly_float phase = phase_ + right_offset + (poly_float(0.5f) & ~constants::kFirstMask) + pair_offset;

            poly_float mod_depth = mod_depth_->buffer[0] * kMaxChorusModulation;
            // Create a sinusoidal modulation of delay time.
            poly_float mod = utils::sin(phase * vital::kPi * 2.0f) * 0.5f + 1.0f;
            float delay_t = 0.0f;
            if (i > 0)
                delay_t = i / (num_voices - 1.0f);

            poly_float delay = mod * mod_depth + utils::interpolate(delay_time, average_delay, delay_t);

            // Set the frequency parameter on the delay line (inverse of delay time).
            vital::poly_float delay_frequency = poly_float(1.0f) / utils::max(0.00001f, delay);
            delay_frequencies_[i].set(delay_frequency);
            // Process the audio through the delay line.
            delays_[i]->processWithInput(audio_out, num_samples);

            // For debugging or analysis, store the delay frequency in the corresponding output.
            delay_status_outputs_[i].buffer[0] = delay_frequency;
        }

        // Manage transitions of wet/dry mixing over the block:
        poly_float current_wet = wet_;
        poly_float current_dry = dry_;

        poly_float wet_value = utils::clamp(wet_output_->buffer[0], 0.0f, 1.0f);
        wet_ = futils::equalPowerFade(wet_value);
        dry_ = futils::equalPowerFadeInverse(wet_value);

        mono_float tick_increment = 1.0f / num_samples;
        poly_float delta_wet = (wet_ - current_wet) * tick_increment;
        poly_float delta_dry = (dry_ - current_dry) * tick_increment;

        // Clear the output buffer before mixing in the delayed signals.
        utils::zeroBuffer(audio_out, num_samples);

        // Mix in all delay outputs to create the chorus effect.
        for (int i = 0; i < num_voices; ++i) {
            const poly_float* delay_out = delays_[i]->output()->buffer;

            for (int s = 0; s < num_samples; ++s) {
                poly_float sample_out = delay_out[s] * 0.5f;
                audio_out[s] += sample_out + utils::swapVoices(sample_out);
            }
        }

        // Apply a gradually changing wet/dry mix to smoothly transition over the block.
        for (int s = 0; s < num_samples; ++s) {
            current_dry += delta_dry;
            current_wet += delta_wet;
            audio_out[s] = current_dry * audio_in[s] + current_wet * audio_out[s];
        }
    }

    void ChorusModule::correctToTime(double seconds) {
        // Recalculate the phase so that the chorus aligns to a given time, for sync or timing-sensitive operations.
        phase_ = utils::getCycleOffsetFromSeconds(seconds, frequency_->buffer[0]);
    }
} // namespace vital
