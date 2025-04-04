#pragma once

#include "synth_module.h"
#include "sample_source.h"

namespace vital {

    /**
     * @brief A module that plays back a sample (audio file) as part of the synthesis pipeline.
     *
     * The SampleModule allows for triggering and controlling sample playback within the
     * synthesizer. It supports parameters such as looping, bouncing (reversing),
     * pitch transposition (with quantization), and level/pan adjustments.
     * It can also respond to MIDI inputs for pitch and note count information.
     */
    class SampleModule : public SynthModule {
    public:
        /**
         * @enum InputIndices
         * @brief Input indices for this module.
         */
        enum {
            kReset,      /**< Input to reset the sample playback position. */
            kMidi,       /**< MIDI input for pitch and note trigger information. */
            kNoteCount,  /**< The current count of active notes. */
            kNumInputs   /**< Total number of inputs. */
        };

        /**
         * @enum OutputIndices
         * @brief Output indices for this module.
         */
        enum {
            kRaw,       /**< Raw sample output before level adjustments. */
            kLevelled,  /**< Output after applying level and pan adjustments. */
            kNumOutputs /**< Total number of outputs. */
        };

        /**
         * @brief Constructs a SampleModule.
         */
        SampleModule();

        /**
         * @brief Destructor.
         */
        virtual ~SampleModule() { }

        /**
         * @brief Processes a block of samples for the given number of samples.
         *
         * If the sample is turned on, it processes audio normally. If turned off,
         * it clears the output buffers and resets the phase output when it changes state.
         *
         * @param num_samples The number of samples to process.
         */
        void process(int num_samples) override;

        /**
         * @brief Initializes the sample module by creating controls and plugging them into the sampler.
         *
         * This includes settings for looping, keytracking, transposition, tuning, level, and pan.
         */
        void init() override;

        /**
         * @brief Creates a clone of this SampleModule.
         *
         * @return A new SampleModule identical to this one.
         */
        virtual Processor* clone() const override { return new SampleModule(*this); }

        /**
         * @brief Gets the Sample object used by the sampler.
         *
         * @return A pointer to the Sample currently in use.
         */
        Sample* getSample() { return sampler_->getSample(); }

        /**
         * @brief Retrieves the phase output of the sampler.
         *
         * @return A pointer to the Output representing the sampler's current phase.
         */
        force_inline Output* getPhaseOutput() const { return sampler_->getPhaseOutput(); }

    protected:
        std::shared_ptr<bool> was_on_; /**< Tracks whether the sample was previously playing. */
        SampleSource* sampler_;        /**< The internal sample source that generates audio from a sample. */
        Value* on_;                    /**< A control value indicating if the sample is currently on (enabled). */

        JUCE_LEAK_DETECTOR(SampleModule)
    };
} // namespace vital
