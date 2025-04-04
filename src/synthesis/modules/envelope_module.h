#pragma once

#include "synth_module.h"

namespace vital {

    class Envelope;

    /**
     * @brief A module that generates an envelope signal (ADSR-like) controlled by various parameters.
     *
     * The EnvelopeModule manages a single Envelope processor, which can be triggered and shaped
     * by parameters such as delay, attack, hold, decay, sustain, and release times, as well as
     * curvature (power) settings for the envelope segments. The module outputs both the envelope
     * value and its phase/state, allowing for complex modulation scenarios.
     */
    class EnvelopeModule : public SynthModule {
    public:
        /**
         * @brief Input indices for the EnvelopeModule.
         *
         * - kTrigger: Used to trigger the envelope (e.g., note on/off).
         */
        enum {
            kTrigger,
            kNumInputs
        };

        /**
         * @brief Output indices for the EnvelopeModule.
         *
         * - kValue: The current envelope value (amplitude) over time.
         * - kPhase: The current phase of the envelope (which segment/stage it is in).
         */
        enum {
            kValue,
            kPhase,
            kNumOutputs
        };

        /**
         * @brief Constructs an EnvelopeModule with a given prefix for parameter naming and a control rate mode.
         *
         * @param prefix A string prefix for all parameter names (e.g., "env").
         * @param force_audio_rate If true, the envelope is processed at audio rate rather than control rate.
         */
        EnvelopeModule(const std::string& prefix, bool force_audio_rate = false);

        /**
         * @brief Destroys the EnvelopeModule.
         */
        virtual ~EnvelopeModule() { }

        /**
         * @brief Initializes the EnvelopeModule, creating parameter controls and connecting them to the internal Envelope.
         */
        void init() override;

        /**
         * @brief Creates a clone of the current EnvelopeModule with identical settings.
         * @return A pointer to the newly cloned EnvelopeModule.
         */
        virtual Processor* clone() const override { return new EnvelopeModule(*this); }

        /**
         * @brief Sets the processing mode of the envelope (control-rate or audio-rate) if not forced.
         *
         * @param control_rate True to run at control rate, false to run at audio rate.
         */
        void setControlRate(bool control_rate) override {
            if (!force_audio_rate_)
                envelope_->setControlRate(control_rate);
        }

    protected:
        std::string prefix_;    ///< Prefix for all envelope parameter names.
        Envelope* envelope_;    ///< The internal Envelope processor handling the envelope generation.
        bool force_audio_rate_; ///< Whether this module is forced to run at audio rate.

        JUCE_LEAK_DETECTOR(EnvelopeModule)
    };
} // namespace vital
