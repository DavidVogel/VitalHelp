#include "random_lfo.h"

#include "synth_lfo.h"
#include "utils.h"
#include "futils.h"
#include "synth_constants.h"

namespace {
    // Lorenz system constants:
    constexpr float kLorenzInitial1 = 0.0f;
    constexpr float kLorenzInitial2 = 0.0f;
    constexpr float kLorenzInitial3 = 37.6f;
    constexpr float kLorenzA = 10.0f;
    constexpr float kLorenzB = 28.0f;
    constexpr float kLorenzC = 8.0f / 3.0f;
    constexpr float kLorenzTimeScale = 1.0f;
    constexpr float kLorenzSize = 40.0f;
    constexpr float kLorenzScale = 1.0f / kLorenzSize;
}

namespace vital {
    RandomLfo::RandomLfo() : Processor(kNumInputs, 1), random_generator_(-1.0f, 1.0f) {
        last_sync_ = std::make_shared<double>();
        sync_seconds_ = std::make_shared<double>();
        shared_state_ = std::make_shared<RandomState>();
        *sync_seconds_ = 0;
    }

    void RandomLfo::doReset(RandomState* state, bool mono, poly_float frequency) {
        /**
         * @brief Handles resetting the LFO if a reset trigger occurs.
         *
         * It checks for a reset trigger and, if present, resets the offset to account for the trigger
         * time, and chooses new random values for the start and end points of the interpolation.
         */
        poly_mask reset_mask = getResetMask(kReset);
        // If no reset trigger or if sync input is active, no reset is performed.
        if (reset_mask.anyMask() == 0 || input(kSync)->at(0)[0])
            return;

        poly_float sample_offset = utils::toFloat(input(kReset)->source->trigger_offset);
        poly_float start_offset = frequency * (1.0f / getSampleRate()) * sample_offset;
        state->offset = utils::maskLoad(state->offset, -start_offset, reset_mask);

        poly_float from_random = 0.0f;
        poly_float to_random = 0.0f;
        // Generate new random values for mono or stereo voices.
        if (mono) {
            from_random = random_generator_.polyVoiceNext();
            to_random = random_generator_.polyVoiceNext();
        }
        else {
            from_random = random_generator_.polyNext();
            to_random = random_generator_.polyNext();
        }

        state->last_random_value = utils::maskLoad(state->last_random_value, from_random, reset_mask);
        state->next_random_value = utils::maskLoad(state->next_random_value, to_random, reset_mask);
        last_value_ = utils::maskLoad(last_value_, state->last_random_value * 0.5f + 0.5f, reset_mask);
    }

    poly_int RandomLfo::updatePhase(RandomState* state, int num_samples) {
        /**
         * @brief Advances the LFO phase by an amount determined by frequency and sample count.
         *
         * If the LFO is sync'd, it sets the offset based on external timing. Otherwise, it moves
         * forward normally and checks if a new random value should be chosen when wrapping around.
         *
         * @return The number of samples until the phase wraps around, or 0 if no wrap occurs.
         */
        poly_float frequency = input(kFrequency)->at(0);
        poly_float phase_delta = frequency * (1.0f / getSampleRate()) * num_samples;
        bool mono = input(kStereo)->at(0)[0] == 0.0f;
        poly_mask new_random_mask = 0;

        if (input(kSync)->at(0)[0]) {
            // Sync mode: If external sync time changes, adjust offset accordingly.
            if (*last_sync_ != *sync_seconds_) {
                poly_float new_offset = utils::getCycleOffsetFromSeconds(*sync_seconds_, frequency);
                new_random_mask = poly_float::lessThan(new_offset, 0.5f) & poly_float::greaterThanOrEqual(state->offset, 0.5f);
                state->offset = new_offset;
            }
        }
        else {
            // Non-sync mode: normal phase increment, with resets if triggered.
            poly_float frequency = input(kFrequency)->at(0);
            doReset(state, mono, frequency);

            state->offset += phase_delta;
            new_random_mask = poly_float::greaterThanOrEqual(state->offset, 1.0f);
            state->offset = utils::mod(state->offset);
        }

        // If the phase passed 1.0 (full cycle), select a new random value pair.
        if (new_random_mask.anyMask()) {
            state->last_random_value = utils::maskLoad(state->last_random_value, state->next_random_value, new_random_mask);
            poly_float next_random = mono ? random_generator_.polyVoiceNext() : random_generator_.polyNext();
            state->next_random_value = utils::maskLoad(state->next_random_value, next_random, new_random_mask);

            // Compute how many samples until wrapping, for accurate timing.
            poly_float delta = utils::maskLoad(phase_delta, 1.0f, poly_float::lessThanOrEqual(phase_delta, 0.0f));
            poly_float samples_to_wrap = state->offset / delta;
            return utils::roundToInt(samples_to_wrap);
        }

        return 0;
    }

    void RandomLfo::process(int num_samples) {
        /**
         * @brief High-level processing entry point.
         *
         * If syncing is enabled and the sync time has changed, processes the shared state. Otherwise,
         * processes the internal state. It also handles control-rate vs. audio-rate interpolation of values.
         */
        if (input(kSync)->at(0)[0]) {
            if (*last_sync_ != *sync_seconds_) {
                // Sync changed, so process with the shared state.
                process(shared_state_.get(), num_samples);

                poly_float* dest = output()->buffer;
                int update_samples = isControlRate() ? 1 : num_samples;
                // Stereo handling: combine left and right values if needed.
                for (int i = 0; i < update_samples; ++i) {
                    poly_float value = dest[i] & constants::kFirstMask;
                    dest[i] = value + utils::swapVoices(value);
                }

                poly_float trigger_value = output()->trigger_value & constants::kFirstMask;
                output()->trigger_value = trigger_value + utils::swapVoices(trigger_value);
                *last_sync_ = *sync_seconds_;
            }
        }
        else
            process(&state_, num_samples);
    }

    void RandomLfo::process(RandomState* state, int num_samples) {
        /**
         * @brief Processes the LFO for a given state without external sync considerations.
         *
         * It selects the random type and processes accordingly. If it's Lorenz or Sample-And-Hold,
         * it delegates to those specific methods. Otherwise, it updates the phase, interpolates random values,
         * and writes the output. On audio-rate processing, it smoothly transitions between the old and new values.
         */
        int random_type_int = std::round(utils::clamp(input(kStyle)->at(0)[0], 0.0f, kNumStyles - 1.0f));
        RandomType random_type = static_cast<RandomType>(random_type_int);

        if (random_type == kLorenzAttractor) {
            processLorenzAttractor(state, num_samples);
            return;
        }
        if (random_type == kSampleAndHold) {
            processSampleAndHold(state, num_samples);
            return;
        }

        updatePhase(state, num_samples);

        poly_float result;
        switch (random_type) {
            case kPerlin:
                result = utils::perlinInterpolate(state->last_random_value, state->next_random_value, state->offset);
                break;
            case kSinInterpolate:
                result = futils::sinInterpolate(state->last_random_value, state->next_random_value, state->offset);
                break;
            default:
                result = 0.0f;
        }

        result = result * 0.5f + 0.5f; // Normalize from [-1, 1] to [0, 1].
        output()->trigger_value = result;

        poly_float* dest = output()->buffer;
        if (!isControlRate()) {
            // Audio-rate: interpolate values smoothly across samples.
            poly_float current_value = last_value_;
            poly_float delta_value = (result - current_value) * (1.0f / num_samples);
            for (int i = 0; i < num_samples; ++i) {
                current_value += delta_value;
                dest[i] = current_value;
            }
        }
        else
            dest[0] = result;

        last_value_ = result;
    }

    void RandomLfo::processSampleAndHold(RandomState* state, int num_samples) {
        /**
         * @brief Processes the LFO in Sample-And-Hold mode.
         *
         * The output stays constant over each cycle until the phase wraps around,
         * at which point a new random value is chosen and held.
         */
        poly_float last_random_value = state->last_random_value * 0.5f + 0.5f;
        poly_int sample_change = updatePhase(state, num_samples);
        poly_float current_random_value = state->last_random_value * 0.5f + 0.5f;

        poly_float* dest = output()->buffer;
        if (!isControlRate()) {
            for (int i = 0; i < num_samples; ++i) {
                poly_mask over = poly_int::greaterThan(i, sample_change);
                dest[i] = utils::maskLoad(last_random_value, current_random_value, over);
            }
        }
        else
            dest[0] = current_random_value;

        output()->trigger_value = current_random_value;
    }

    void RandomLfo::processLorenzAttractor(RandomState* state, int num_samples) {
        /**
         * @brief Processes the LFO using the Lorenz attractor.
         *
         * The Lorenz system generates a chaotic waveform. This method integrates the Lorenz equations
         * over time to produce a varying output. The output is normalized to fit within [0, 1].
         */
        static constexpr float kMaxFrequency = 0.01f;

        bool mono = input(kStereo)->at(0)[0] == 0.0f;
        poly_float state1 = state->state1;
        poly_float state2 = state->state2;
        poly_float state3 = state->state3;

        poly_mask reset_mask = getResetMask(kReset);
        if (reset_mask.anyMask() && input(kSync)->at(0)[0] == 0.0f) {
            // On reset, re-initialize states with random offsets if not synced.
            if (mono) {
                poly_float value1 = random_generator_.polyVoiceNext() + kLorenzInitial1;
                poly_float value2 = random_generator_.polyVoiceNext() + kLorenzInitial2;
                poly_float value3 = random_generator_.polyVoiceNext() + kLorenzInitial3;
                state1 = utils::maskLoad(state1, value1, reset_mask);
                state2 = utils::maskLoad(state2, value2, reset_mask);
                state3 = utils::maskLoad(state3, value3, reset_mask);
            }
            else {
                poly_float value1 = random_generator_.polyNext() + kLorenzInitial1;
                poly_float value2 = random_generator_.polyNext() + kLorenzInitial2;
                poly_float value3 = random_generator_.polyNext() + kLorenzInitial3;
                state1 = utils::maskLoad(state1, value1, reset_mask);
                state2 = utils::maskLoad(state2, value2, reset_mask);
                state3 = utils::maskLoad(state3, value3, reset_mask);
            }
        }

        if (mono) {
            // In mono mode, ensure states are consistent across stereo pairs if any.
            state1 = state1 & constants::kLeftMask;
            state1 += utils::swapStereo(state1);
            state2 = state2 & constants::kLeftMask;
            state2 += utils::swapStereo(state2);
            state3 = state3 & constants::kLeftMask;
            state3 += utils::swapStereo(state3);
        }

        poly_float frequency = input(kFrequency)->at(0);
        poly_float t = utils::min(kMaxFrequency, frequency * (0.5f / getSampleRate()));

        poly_float* dest = output()->buffer;
        // Integrate the Lorenz system:
        for (int i = 0; i < num_samples; ++i) {
            poly_float delta1 = (state2 - state1) * kLorenzA;
            poly_float delta2 = (-state3 + kLorenzB) * state1 - state2;
            poly_float delta3 = state1 * state2 - state3 * kLorenzC;
            state1 += delta1 * t;
            state2 += delta2 * t;
            state3 += delta3 * t;

            dest[i] = state1 * kLorenzScale + 0.5f;
        }

        state->state1 = state1;
        state->state2 = state2;
        state->state3 = state3;

        output()->trigger_value = state->state1 * kLorenzScale + 0.5f;
    }

    void RandomLfo::correctToTime(double seconds) {
        /**
         * @brief Adjusts the LFO to align with a given time in seconds, enabling tempo sync.
         */
        *sync_seconds_ = seconds;
    }
} // namespace vital
