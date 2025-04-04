#pragma once

#include "synth_module.h"

namespace vital {

    class RandomLfo;

    /**
     * @brief A module that produces random low-frequency oscillations (LFOs) for modulation purposes.
     *
     * The RandomLfoModule generates random LFO signals that can be synced to tempo or run freely.
     * It offers parameters for controlling the LFO style, frequency, stereo spread, and synchronization behavior.
     * When triggered by note events, the LFO can reset to ensure consistency on each note start.
     */
    class RandomLfoModule : public SynthModule {
    public:
        /**
         * @enum InputIndices
         * @brief Defines the input indices for this module.
         */
        enum {
            kNoteTrigger, /**< A trigger input that resets the LFO on a new note. */
            kMidi,        /**< MIDI input for potential frequency or sync modulation. */
            kNumInputs    /**< Total number of inputs. */
        };

        /**
         * @brief Constructs a RandomLfoModule with a given prefix and a beats-per-second reference.
         *
         * @param prefix A string prefix used for naming the LFO parameters.
         * @param beats_per_second A pointer to an Output providing the current tempo in beats per second.
         */
        RandomLfoModule(const std::string& prefix, const Output* beats_per_second);

        /**
         * @brief Destructor.
         */
        virtual ~RandomLfoModule() { }

        /**
         * @brief Initializes the RandomLfoModule by creating and connecting control parameters to the internal RandomLfo.
         *
         * This includes frequency (either free or tempo-synced), style, stereo, and sync type settings.
         */
        void init() override;

        /**
         * @brief Creates a new RandomLfoModule identical to this one.
         *
         * @return A new RandomLfoModule instance that is a clone of this one.
         */
        virtual Processor* clone() const override { return new RandomLfoModule(*this); }

        /**
         * @brief Adjusts the LFO to a given time position.
         *
         * This ensures that the LFO phase and internal timing align with the provided playback time.
         *
         * @param seconds The playback time in seconds to which to align the LFO.
         */
        void correctToTime(double seconds) override;

    protected:
        std::string prefix_;           /**< Prefix used for naming LFO parameters and controls. */
        RandomLfo* lfo_;               /**< The internal RandomLfo processor generating the LFO signal. */
        const Output* beats_per_second_; /**< Output providing a tempo reference in beats per second for sync. */

        JUCE_LEAK_DETECTOR(RandomLfoModule)
    };
} // namespace vital
