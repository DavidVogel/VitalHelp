#pragma once

#include "synth_module.h"
#include "synth_oscillator.h"

namespace vital {
    class Wavetable;

    /**
     * @brief A synthesis module that represents an oscillator within the Vital synthesizer.
     *
     * This module manages a SynthOscillator instance which generates audio waveforms.
     * It provides control over wavetable position, pitch, amplitude, phase, and various
     * modulation parameters, as well as distortion and spectral morphing functionality.
     */
    class OscillatorModule : public SynthModule {
    public:
        /**
         * @enum InputIndices
         * @brief Defines the input index mapping for this module.
         */
        enum {
            kReset,        /**< Input for resetting oscillator phases. */
            kRetrigger,    /**< Input to trigger oscillator retriggers. */
            kMidi,         /**< MIDI note input for pitch tracking. */
            kActiveVoices, /**< Input representing currently active voices. */
            kNumInputs     /**< Total number of inputs. */
        };

        /**
         * @enum OutputIndices
         * @brief Defines the output index mapping for this module.
         */
        enum {
            kRaw,      /**< Raw waveform output before amplitude leveling. */
            kLevelled, /**< Leveled waveform output after amplitude processing. */
            kNumOutputs /**< Total number of outputs. */
        };

        /**
         * @brief Constructs an OscillatorModule with an optional prefix for parameter naming.
         *
         * @param prefix A string prefix used for naming parameters and controls. Defaults to an empty string.
         */
        OscillatorModule(std::string prefix = "");

        /**
         * @brief Destructor.
         */
        virtual ~OscillatorModule() { }

        /**
         * @brief Processes the oscillator for a given number of samples.
         *
         * Depending on the "on" parameter, this processes the audio or clears the output if turned off.
         *
         * @param num_samples The number of samples to process.
         */
        void process(int num_samples) override;

        /**
         * @brief Initializes the module and sets up the internal SynthOscillator with appropriate parameters.
         *
         * This includes creating modulation controls and connecting them to the oscillator.
         */
        void init() override;

        /**
         * @brief Creates a clone of this oscillator module.
         *
         * @return A new instance of OscillatorModule identical to this one.
         */
        virtual Processor* clone() const override { return new OscillatorModule(*this); }

        /**
         * @brief Returns the associated Wavetable object.
         *
         * @return A pointer to the Wavetable used by the oscillator.
         */
        Wavetable* getWavetable() { return wavetable_.get(); }

        /**
         * @brief Returns a pointer to the internal SynthOscillator.
         *
         * @return A pointer to the SynthOscillator instance.
         */
        force_inline SynthOscillator* oscillator() { return oscillator_; }

        /**
         * @brief Gets the currently set distortion type.
         *
         * @return The distortion type, as a SynthOscillator::DistortionType enum value.
         */
        SynthOscillator::DistortionType getDistortionType() {
            int val = distortion_type_->value();
            return static_cast<SynthOscillator::DistortionType>(val);
        }

    protected:
        std::string prefix_;                 /**< Prefix used for parameter naming. */
        std::shared_ptr<Wavetable> wavetable_; /**< Shared pointer to the Wavetable used by the oscillator. */
        std::shared_ptr<bool> was_on_;         /**< Tracks whether the oscillator was on in the previous cycle. */

        Value* on_;                        /**< Control value determining if the oscillator is enabled. */
        SynthOscillator* oscillator_;      /**< The internal SynthOscillator generating audio. */
        Value* distortion_type_;           /**< Control for selecting the distortion type. */

        JUCE_LEAK_DETECTOR(OscillatorModule)
    };
} // namespace vital
