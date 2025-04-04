#pragma once

#include "processor.h"
#include "line_generator.h"

namespace vital {

    /**
     * @brief A versatile Low-Frequency Oscillator (LFO) for audio synthesis, supporting multiple sync modes and smoothing options.
     *
     * The SynthLfo class generates low-frequency modulation signals from a LineGenerator source. It supports:
     * - Various sync types (triggered, synced to host/tempo, envelope-like, looping)
     * - Phase offset adjustments, stereo offsets, delay times, fade-in times, and smoothing.
     * - Both control-rate and audio-rate processing, adapting seamlessly between them.
     */
    class SynthLfo : public Processor {
    public:
        /**
         * @brief Indices of input parameters to the LFO.
         *
         * - kFrequency:     Controls the speed (frequency) of the LFO cycle.
         * - kPhase:         Sets a base phase offset for the LFO.
         * - kAmplitude:     Adjusts the output amplitude (scaling the final LFO value).
         * - kNoteTrigger:   Triggers the LFO, resetting phase or starting envelopes depending on sync type.
         * - kSyncType:      Determines the synchronization mode (trigger, sync, envelope, loop, etc.).
         * - kSmoothMode:    Enables smoothing of the LFO output using a half-life parameter.
         * - kFade:          Controls a fade-in time for the LFO after being triggered.
         * - kSmoothTime:    Sets the smoothing time (half-life) for transitions.
         * - kStereoPhase:   Applies a stereo phase offset between left/right channels.
         * - kDelay:         Sets a delay time before the LFO starts outputting values.
         * - kNoteCount:     Indicates how many notes/voices are currently active for voice encoding.
         */
        enum {
            kFrequency,
            kPhase,
            kAmplitude,
            kNoteTrigger,
            kSyncType,
            kSmoothMode,
            kFade,
            kSmoothTime,
            kStereoPhase,
            kDelay,
            kNoteCount,
            kNumInputs
        };

        /**
         * @brief Indices of output parameters from the LFO.
         *
         * - kValue:        The LFO's main output value (the modulating signal).
         * - kOscPhase:     The encoded current phase and voice information.
         * - kOscFrequency: The current frequency of the LFO in Hz.
         */
        enum {
            kValue,
            kOscPhase,
            kOscFrequency,
            kNumOutputs
        };

        /**
         * @brief Different synchronization modes for the LFO.
         *
         * - kTrigger:          Triggers the LFO on note start, repeating cycles.
         * - kSync:             Syncs LFO phase to a global time reference.
         * - kEnvelope:         LFO acts as a one-shot envelope, stopping after reaching the end.
         * - kSustainEnvelope:  Envelope-like, but can be held at a certain phase until note release.
         * - kLoopPoint:        Loops from the end back to a specified loop point, creating a custom cycle.
         * - kLoopHold:         Loops and holds at a point when triggered, producing interesting loop-hold behavior.
         */
        enum SyncType {
            kTrigger,
            kSync,
            kEnvelope,
            kSustainEnvelope,
            kLoopPoint,
            kLoopHold,
            kNumSyncTypes
        };

        /**
         * @brief Different time interpretation modes for synchronizing the LFO.
         *
         * Not all are necessarily used here, but it's a reference for future expansions:
         * - kTime:         Use absolute time.
         * - kTempo:        Sync to tempo as normal quarters.
         * - kDottedTempo:  Sync to dotted rhythm times.
         * - kTripletTempo: Sync to triplet rhythm times.
         * - kKeytrack:     Key-tracks frequency based on note pitch.
         */
        enum {
            kTime,
            kTempo,
            kDottedTempo,
            kTripletTempo,
            kKeytrack,
            kNumSyncOptions
        };

        /**
         * @brief Holds the state of the LFO for either control-rate or audio-rate processing.
         *
         * Each LfoState stores timing and smoothing variables to ensure consistent and correct LFO behavior
         * after triggers, fades, and delays.
         */
        struct LfoState {
            poly_float delay_time_passed = 0.0f; ///< How much time has passed since the LFO was triggered or started its delay.
            poly_float fade_amplitude = 0.0f;    ///< The current fade-in amplitude value.
            poly_float smooth_value = 0.0f;      ///< The stored value for applying smoothing between updates.
            poly_float fade_amount = 0.0f;       ///< Controls how much fade has been applied (not directly updated here).
            poly_float offset = 0.0f;            ///< The current LFO offset (phase offset).
            poly_float phase = 0.0f;             ///< The current LFO phase.
        };

        static constexpr mono_float kMaxPower = 20.0f;
        static constexpr float kHalfLifeRatio = 0.2f;
        static constexpr float kMinHalfLife = 0.0002f;

        /**
         * @brief Retrieves the LFO value at a given phase using a cubic interpolation on the line generator data.
         *
         * @param buffer      Pointer to the cubic interpolation buffer from the LineGenerator.
         * @param resolution  The number of samples in the wave table.
         * @param max_index   The maximum valid index in the buffer.
         * @param phase       The normalized phase (0.0 to 1.0) at which to sample.
         * @return The interpolated LFO value at the given phase.
         */
        force_inline poly_float getValueAtPhase(mono_float* buffer, poly_float resolution,
                                                poly_int max_index, poly_float phase) {
            poly_float boost = utils::clamp(phase * resolution, 0.0f, resolution);
            poly_int indices = utils::clamp(utils::toInt(boost), 0, max_index);
            poly_float t = boost - utils::toFloat(indices);

            matrix interpolation_matrix = utils::getCatmullInterpolationMatrix(t);
            matrix value_matrix = utils::getValueMatrix(buffer, indices);
            value_matrix.transpose();

            return interpolation_matrix.multiplyAndSumRows(value_matrix);
        }

        /**
         * @brief Retrieves the LFO value at a given phase using the internal line generator source.
         *
         * @param phase The normalized phase at which to sample.
         * @return The interpolated LFO value at that phase.
         */
        force_inline poly_float getValueAtPhase(poly_float phase) {
            int resolution = source_->resolution();
            return getValueAtPhase(source_->getCubicInterpolationBuffer(), resolution, resolution - 1, phase);
        }

        /**
         * @brief Constructs a SynthLfo processor with a given LineGenerator source.
         * @param source Pointer to a LineGenerator that provides the waveform data for the LFO.
         */
        SynthLfo(LineGenerator* source);

        /**
         * @brief Determines which voices are in the release state based on note triggers.
         * @return A mask of voices that are currently releasing.
         */
        force_inline poly_mask getReleaseMask() {
            poly_mask trigger_mask = input(kNoteTrigger)->source->trigger_mask;
            poly_float trigger_value = input(kNoteTrigger)->source->trigger_value;
            return trigger_mask & poly_float::equal(trigger_value, kVoiceOff);
        }

        /**
         * @brief Clones the SynthLfo processor, creating a new instance with the same settings.
         * @return A pointer to the cloned SynthLfo instance.
         */
        virtual Processor* clone() const override { return new SynthLfo(*this); }

        /**
         * @brief Processes a block of samples, updating the LFO output.
         *
         * Automatically switches between control-rate and audio-rate processing. It handles triggers, fades,
         * smoothing, and various synchronization modes.
         *
         * @param num_samples The number of samples to process in the current block.
         */
        void process(int num_samples) override;

        /**
         * @brief Updates the LFO to align with a given time in seconds, enabling synchronization with an external clock.
         *
         * @param seconds The time in seconds to sync the LFO's phase offset to.
         */
        void correctToTime(double seconds);

    protected:
        /**
         * @brief Handles trigger events (note on/off), resets, and updates masks for held states.
         */
        void processTrigger();

        /**
         * @brief Processes the LFO at control rate (e.g., once per block) instead of every sample.
         *
         * @param num_samples The number of samples in the current block.
         */
        void processControlRate(int num_samples);

        /**
         * @brief Processes the LFO in "Envelope" sync mode at audio rate.
         *
         * In Envelope mode, the LFO runs from 0 to 1 and then stops. This method updates the output sample-by-sample.
         */
        poly_float processAudioRateEnvelope(int num_samples, poly_float current_phase,
                                            poly_float current_offset, poly_float delta_offset);

        /**
         * @brief Processes the LFO in "Sustain Envelope" mode at audio rate.
         *
         * This mode behaves like an envelope but can hold at a certain phase until a release event.
         */
        poly_float processAudioRateSustainEnvelope(int num_samples, poly_float current_phase,
                                                   poly_float current_offset, poly_float delta_offset);

        /**
         * @brief Processes the LFO in regular LFO mode (Trigger or Sync) at audio rate.
         *
         * This mode continuously loops the waveform from 0 to 1.
         */
        poly_float processAudioRateLfo(int num_samples, poly_float current_phase,
                                       poly_float current_offset, poly_float delta_offset);

        /**
         * @brief Processes the LFO in "LoopPoint" mode at audio rate.
         *
         * This mode loops the LFO and after reaching 1.0, jumps back to a loop point offset.
         */
        poly_float processAudioRateLoopPoint(int num_samples, poly_float current_phase,
                                             poly_float current_offset, poly_float delta_offset);

        /**
         * @brief Processes the LFO in "LoopHold" mode at audio rate.
         *
         * In this mode, the LFO loops and holds at a specified phase until triggered again, producing a loop-hold effect.
         */
        poly_float processAudioRateLoopHold(int num_samples, poly_float current_phase,
                                            poly_float current_offset, poly_float delta_offset);

        /**
         * @brief Processes the LFO at audio rate (per sample).
         *
         * This method selects the appropriate sync mode and processing function based on the current configuration.
         */
        void processAudioRate(int num_samples);

        bool was_control_rate_;       ///< Tracks if the previous processing block was at control rate.
        LfoState control_rate_state_; ///< State used during control-rate processing.
        LfoState audio_rate_state_;   ///< State used during audio-rate processing.

        poly_mask held_mask_;         ///< A mask indicating which voices are currently held (for sustain envelope modes).
        poly_int trigger_sample_;     ///< The sample index at which the note was triggered.
        poly_float trigger_delay_;    ///< The delay (in seconds) to apply before the LFO starts after a trigger.

        LineGenerator* source_;       ///< The LineGenerator providing the waveform data for the LFO.

        std::shared_ptr<double> sync_seconds_; ///< Shared pointer to an external time reference for syncing.

        JUCE_LEAK_DETECTOR(SynthLfo)
    };
} // namespace vital
