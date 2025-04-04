#pragma once

#include "processor.h"
#include "utils.h"

namespace vital {

    /**
     * @brief A Low-Frequency Oscillator (LFO) that generates random modulation signals.
     *
     * The RandomLfo class creates a variety of random waveforms, including Perlin-like noise,
     * sample-and-hold stepped values, sinusoidal interpolations, and chaotic Lorenz attractors.
     * It supports resetting on triggers, stereo or mono modes, frequency control, and optional
     * tempo synchronization via an external time reference.
     */
    class RandomLfo : public Processor {
    public:
        /**
         * @brief Holds the internal state of the RandomLfo for a given voice or channel.
         *
         * This structure stores offsets, last and next random values, and various intermediate states
         * required for generating continuous or discrete random waveforms.
         */
        struct RandomState {
            RandomState() {
                offset = 0.0f;
                last_random_value = 0.0f;
                next_random_value = 0.0f;
                state1 = 0.1f;
                state2 = 0.0f;
                state3 = 0.0f;
            }

            poly_float offset;              ///< Current offset (phase) in the LFO cycle.
            poly_float last_random_value;   ///< The previously generated random value.
            poly_float next_random_value;   ///< The next target random value for interpolation.

            // Used by certain random types (like Lorenz attractor) to store state variables.
            poly_float state1, state2, state3;
        };

        /**
         * @brief Input parameter indices for the RandomLfo.
         *
         * - kFrequency: The LFO's frequency.
         * - kAmplitude: The amplitude or scaling factor of the output.
         * - kReset:     A trigger that resets the LFO state.
         * - kSync:      Enables syncing of the LFO to an external time reference.
         * - kStyle:     Determines the type of random waveform (Perlin, Sample & Hold, etc.).
         * - kRandomType: Another parameter that selects random generation style (currently matches kStyle).
         * - kStereo:    Determines if LFO is mono or stereo. In stereo, voices may differ.
         */
        enum {
            kFrequency,
            kAmplitude,
            kReset,
            kSync,
            kStyle,
            kRandomType,
            kStereo,
            kNumInputs
        };

        /**
         * @brief The types of random waveforms supported by RandomLfo.
         *
         * - kPerlin:         Smooth noise (Perlin-like interpolation between random values).
         * - kSampleAndHold:  Stepped random values that change on each cycle.
         * - kSinInterpolate: Random values with sinusoidal interpolation between them.
         * - kLorenzAttractor: A chaotic waveform derived from the Lorenz attractor system.
         */
        enum RandomType {
            kPerlin,
            kSampleAndHold,
            kSinInterpolate,
            kLorenzAttractor,
            kNumStyles
        };

        /**
         * @brief Constructs a RandomLfo processor with default parameters.
         */
        RandomLfo();

        /**
         * @brief Clones the RandomLfo, creating an identical instance.
         * @return A pointer to the newly cloned RandomLfo.
         */
        virtual Processor* clone() const override { return new RandomLfo(*this); }

        /**
         * @brief Processes a block of samples.
         *
         * Depending on control-rate or audio-rate processing, this method updates the LFO output.
         * It also checks if synchronization or resetting is needed and applies the selected random style.
         *
         * @param num_samples The number of samples to process in the current block.
         */
        void process(int num_samples) override;

        /**
         * @brief Processes the LFO using the given state.
         *
         * This is a helper function for handling shared states between voices or instances.
         * It updates random values, phases, and interpolations based on the configured style.
         *
         * @param state       Pointer to a RandomState to update.
         * @param num_samples The number of samples to process.
         */
        void process(RandomState* state, int num_samples);

        /**
         * @brief Processes the LFO in Sample-And-Hold mode.
         *
         * In Sample-And-Hold mode, the LFO outputs a constant random value until the next cycle,
         * when it suddenly jumps to a new random value.
         *
         * @param state       Pointer to the RandomState to update.
         * @param num_samples The number of samples to process.
         */
        void processSampleAndHold(RandomState* state, int num_samples);

        /**
         * @brief Processes the LFO using a Lorenz attractor model.
         *
         * The Lorenz attractor produces chaotic waveforms influenced by nonlinear dynamics.
         * This method simulates a small step of the Lorenz system for each sample.
         *
         * @param state       Pointer to the RandomState to update.
         * @param num_samples The number of samples to process.
         */
        void processLorenzAttractor(RandomState* state, int num_samples);

        /**
         * @brief Adjusts the LFO to match a specific time reference (in seconds), for synchronization.
         *
         * @param seconds The time offset in seconds to which the LFO should sync.
         */
        void correctToTime(double seconds);

    protected:
        /**
         * @brief Resets the LFO phase and random values if a reset trigger occurs.
         *
         * @param state    The RandomState to modify.
         * @param mono     True if LFO is mono, false if stereo.
         * @param frequency The LFO frequency used to determine phase increments.
         */
        void doReset(RandomState* state, bool mono, poly_float frequency);

        /**
         * @brief Updates the LFO phase and determines if a new random value is needed.
         *
         * It increments the phase based on the frequency and sample count.
         * When the phase wraps past 1.0, a new random value is selected.
         *
         * @param state       The RandomState being processed.
         * @param num_samples The number of samples to process.
         * @return An integer representing how many samples until a wrap occurs (for timing).
         */
        poly_int updatePhase(RandomState* state, int num_samples);

        RandomState state_;                           ///< The main internal RandomState for this LFO instance.
        std::shared_ptr<RandomState> shared_state_;    ///< A shared RandomState (used when syncing across instances).

        utils::RandomGenerator random_generator_;      ///< A random generator to produce random values for the LFO.
        poly_float last_value_;                        ///< The last output value of the LFO.

        std::shared_ptr<double> sync_seconds_;         ///< A shared double holding the sync reference time in seconds.
        std::shared_ptr<double> last_sync_;            ///< A shared double holding the last sync time for comparison.

        JUCE_LEAK_DETECTOR(RandomLfo)
    };
} // namespace vital
