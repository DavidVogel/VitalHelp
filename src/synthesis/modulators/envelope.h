#pragma once

#include "processor.h"
#include "utils.h"

namespace vital {

    /**
     * @brief A processor that generates an envelope signal based on typical ADSR (Attack, Decay, Sustain, Release) parameters.
     *
     * The Envelope class controls the amplitude of a signal over time based on configurable
     * segments (Delay, Attack, Hold, Decay, Sustain, and Release). It inherits from Processor,
     * allowing it to be integrated into a larger signal processing chain. The envelope can be
     * triggered and retriggered, transitioning through states that define its shape.
     */
    class Envelope : public Processor {
    public:
        /**
         * @brief Input parameter indices for the Envelope processor.
         *
         * - kDelay:    The delay time before the envelope starts rising.
         * - kAttack:   The time taken for the envelope to rise from 0 to its peak value.
         * - kAttackPower: A curvature parameter for the attack phase.
         * - kHold:     The time the envelope stays at its peak level after attack.
         * - kDecay:    The time taken for the envelope to fall from its peak to the sustain level.
         * - kDecayPower: A curvature parameter for the decay phase.
         * - kSustain:  The level that the envelope holds after decay until release is triggered.
         * - kRelease:  The time taken for the envelope to fall from the sustain level back to 0 after release is triggered.
         * - kReleasePower: A curvature parameter for the release phase.
         * - kTrigger:  Triggers the envelope (e.g., note on/off events).
         */
        enum {
            kDelay,
            kAttack,
            kAttackPower,
            kHold,
            kDecay,
            kDecayPower,
            kSustain,
            kRelease,
            kReleasePower,
            kTrigger,
            kNumInputs
        };

        /**
         * @brief Output indices for the Envelope processor.
         *
         * - kValue: The current amplitude value of the envelope.
         * - kPhase: The current phase/state of the envelope (including position within a segment).
         */
        enum ProcessorOutput {
            kValue,
            kPhase,
            kNumOutputs
        };

        /**
         * @brief Constructs a new Envelope processor with default parameters.
         */
        Envelope();

        /**
         * @brief Destroys the Envelope processor.
         */
        virtual ~Envelope() { }

        /**
         * @brief Clones the Envelope processor, creating a new instance with identical settings.
         * @return A pointer to the newly cloned Envelope instance.
         */
        virtual Processor* clone() const override { return new Envelope(*this); }

        /**
         * @brief Processes a block of samples. The processing mode (control-rate or audio-rate)
         *        is determined by the parent Processor class.
         *
         * @param num_samples The number of samples to process.
         */
        virtual void process(int num_samples) override;

    private:
        /**
         * @brief Processes the envelope in control-rate mode. This is typically used when
         *        parameter updates or triggers occur at a lower rate than audio.
         *
         * @param num_samples The number of samples corresponding to one control block.
         */
        void processControlRate(int num_samples);

        /**
         * @brief Processes the envelope in audio-rate mode. This updates the envelope
         *        sample-by-sample at audio rate.
         *
         * @param num_samples The number of audio samples to process.
         */
        void processAudioRate(int num_samples);

        /**
         * @brief Processes one segment (section) of the envelope with given parameters.
         *
         * This function interpolates envelope values over a portion of the sample buffer.
         * It calculates and applies shape modifications (power curves) to the envelope
         * segment between specified start and end values.
         *
         * @param audio_out    Pointer to the output buffer where envelope values are stored.
         * @param from         Starting sample index in the buffer.
         * @param to           Ending sample index in the buffer.
         * @param power        The power (curvature) applied to the envelope shaping.
         * @param delta_power  The change in power per sample, used for continuously varying envelope shapes.
         * @param position     The starting position within the envelope segment.
         * @param delta_position The increment per sample in the position.
         * @param start        The starting envelope level.
         * @param end          The ending envelope level.
         * @param delta_end    The change per sample in the ending envelope level.
         * @return The updated position after processing the given segment.
         */
        poly_float processSection(poly_float* audio_out, int from, int to,
                                  poly_float power, poly_float delta_power,
                                  poly_float position, poly_float delta_position,
                                  poly_float start, poly_float end, poly_float delta_end);

        poly_float current_value_;   ///< The current envelope output value.

        poly_float position_;        ///< The normalized position within the current envelope segment.
        poly_float value_;           ///< The envelope value last computed.
        poly_float poly_state_;      ///< The state of the envelope (idle, attack, hold, decay, release, etc.) in SIMD form.
        poly_float start_value_;     ///< The envelope value at the start of a new stage.

        poly_float attack_power_;    ///< The power (curve) parameter for the attack segment.
        poly_float decay_power_;     ///< The power (curve) parameter for the decay segment.
        poly_float release_power_;   ///< The power (curve) parameter for the release segment.
        poly_float sustain_;         ///< The sustain level of the envelope.

        JUCE_LEAK_DETECTOR(Envelope)
    };
} // namespace vital
