#pragma once

#include "synth_module.h"
#include "oscillator_module.h"
#include "sample_module.h"

namespace vital {

    /**
     * @brief A module that manages multiple audio producers (oscillators and sampler) and routes their outputs.
     *
     * The ProducersModule coordinates a set of oscillators and a sampler to produce audio signals that are then
     * routed to various destinations, such as filters, effects, or direct outputs. It also handles modulation
     * dependencies between oscillators, ensuring the correct order of processing based on oscillator modulation
     * and distortion types.
     */
    class ProducersModule : public SynthModule {
    public:
        /**
         * @enum InputIndices
         * @brief Defines the input indices for this module.
         */
        enum {
            kReset,        /**< A reset input to initialize or reset internal states of producers. */
            kRetrigger,    /**< A retrigger input to restart oscillators at certain points in time. */
            kMidi,         /**< MIDI input for pitch and modulation tracking. */
            kActiveVoices, /**< Current number of active voices input. */
            kNoteCount,    /**< Input representing the number of currently held notes. */
            kNumInputs     /**< Total number of inputs. */
        };

        /**
         * @enum OutputIndices
         * @brief Defines the output indices for this module.
         */
        enum {
            kToFilter1,  /**< Output routed to the first filter path. */
            kToFilter2,  /**< Output routed to the second filter path. */
            kRawOut,     /**< A raw output for effects processing or mixing. */
            kDirectOut,  /**< Direct output bypassing filters and effects. */
            kNumOutputs  /**< Total number of outputs. */
        };

        /**
         * @brief Helper function to determine the first modulation index for a given oscillator index.
         *
         * @param index The oscillator index.
         * @return The first modulation index related to the given oscillator.
         */
        static force_inline int getFirstModulationIndex(int index) {
            return index == 0 ? 1 : 0;
        }

        /**
         * @brief Helper function to determine the second modulation index for a given oscillator index.
         *
         * @param index The oscillator index.
         * @return The second modulation index related to the given oscillator.
         */
        static force_inline int getSecondModulationIndex(int index) {
            return index == 1 ? 2 : (getFirstModulationIndex(index) + 1);
        }

        /**
         * @brief Constructs a ProducersModule.
         *
         * Initializes multiple oscillators and a sample module, preparing them for routing and processing.
         */
        ProducersModule();

        /**
         * @brief Destructor.
         */
        virtual ~ProducersModule() { }

        /**
         * @brief Processes the audio for a given number of samples.
         *
         * Orchestrates the correct processing order of oscillators and sampler based on modulation dependencies.
         * Summation of oscillator and sample outputs are routed to the appropriate destinations (filters, raw out, etc.).
         *
         * @param num_samples The number of audio samples to process.
         */
        void process(int num_samples) override;

        /**
         * @brief Initializes the module and sets up parameter connections.
         *
         * Establishes connections to the inputs, sets destinations for oscillators and sampler, and
         * configures modulation references.
         */
        void init() override;

        /**
         * @brief Clones this ProducersModule.
         *
         * @return A new ProducersModule identical to the original.
         */
        virtual Processor* clone() const override { return new ProducersModule(*this); }

        /**
         * @brief Retrieves the Wavetable associated with a specified oscillator.
         *
         * @param index The oscillator index.
         * @return A pointer to the Wavetable for the given oscillator.
         */
        Wavetable* getWavetable(int index) {
            return oscillators_[index]->getWavetable();
        }

        /**
         * @brief Retrieves the current Sample used by the sampler.
         *
         * @return A pointer to the Sample object.
         */
        Sample* getSample() { return sampler_->getSample(); }

        /**
         * @brief Gets the output phase of the sampler.
         *
         * @return The Output associated with the sampler's phase.
         */
        Output* samplePhaseOutput() { return sampler_->getPhaseOutput(); }

        /**
         * @brief Sets a Value that determines if Filter 1 is on.
         *
         * @param on A pointer to a Value indicating if Filter 1 should be enabled.
         */
        void setFilter1On(const Value* on) { filter1_on_ = on; }

        /**
         * @brief Sets a Value that determines if Filter 2 is on.
         *
         * @param on A pointer to a Value indicating if Filter 2 should be enabled.
         */
        void setFilter2On(const Value* on) { filter2_on_ = on; }

    protected:
        /**
         * @brief Checks if Filter 1 is on.
         *
         * @return True if Filter 1 is on, false otherwise.
         */
        bool isFilter1On() { return filter1_on_ == nullptr || filter1_on_->value() != 0.0f; }

        /**
         * @brief Checks if Filter 2 is on.
         *
         * @return True if Filter 2 is on, false otherwise.
         */
        bool isFilter2On() { return filter2_on_ == nullptr || filter2_on_->value() != 0.0f; }

        /**
         * @brief An array of oscillator modules managed by this ProducersModule.
         */
        OscillatorModule* oscillators_[kNumOscillators];

        /**
         * @brief An array of Values determining the destination for each oscillator.
         *
         * The destination influences whether the oscillator output goes to filters, raw output, or direct output.
         */
        Value* oscillator_destinations_[kNumOscillators];

        /**
         * @brief A Value determining the output destination for the sample.
         */
        Value* sample_destination_;

        /**
         * @brief The sampler module that provides sampled audio.
         */
        SampleModule* sampler_;

        /**
         * @brief A Value controlling the state of Filter 1.
         */
        const Value* filter1_on_;

        /**
         * @brief A Value controlling the state of Filter 2.
         */
        const Value* filter2_on_;

        JUCE_LEAK_DETECTOR(ProducersModule)
    };
} // namespace vital
