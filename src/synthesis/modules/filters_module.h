#pragma once

#include "synth_module.h"
#include "filter_module.h"

namespace vital {

    /**
     * @brief A module that manages two filter modules and provides multiple routing configurations.
     *
     * The FiltersModule combines two instances of FilterModule and allows them to be connected in
     * parallel, serial (forward), or serial (backward) configurations. The input signals to each
     * filter and the output routing mode (parallel or serial) are determined by control parameters,
     * enabling dynamic and complex filter topologies.
     */
    class FiltersModule : public SynthModule {
    public:
        /**
         * @brief Input parameter indices for the FiltersModule.
         *
         * - kFilter1Input: The audio input that primarily feeds into filter 1.
         * - kFilter2Input: The audio input that primarily feeds into filter 2.
         * - kKeytrack:     A keytrack input that can be routed to the filters for pitch-dependent cutoff tracking.
         * - kMidi:         MIDI-related input for advanced filter controls (e.g., note on/off).
         * - kReset:        A reset trigger to reinitialize filter states.
         */
        enum {
            kFilter1Input,
            kFilter2Input,
            kKeytrack,
            kMidi,
            kReset,
            kNumInputs
        };

        /**
         * @brief Constructs the FiltersModule, initializing internal states and creating outputs for filter inputs.
         */
        FiltersModule();
        virtual ~FiltersModule() { }

        /**
         * @brief Processes a block of samples, applying either parallel or serial filter configurations based on parameters.
         *
         * This method determines whether filters are routed in parallel or serial mode (forward or backward)
         * and processes the input audio accordingly.
         *
         * @param num_samples The number of audio samples to process.
         */
        void process(int num_samples) override;

        /**
         * @brief Processes both filters in parallel, mixing their outputs.
         *
         * @param num_samples The number of samples in the current block.
         */
        void processParallel(int num_samples);

        /**
         * @brief Processes filters in serial mode, with filter 1 feeding into filter 2.
         *
         * @param num_samples The number of samples in the current block.
         */
        void processSerialForward(int num_samples);

        /**
         * @brief Processes filters in the reverse serial mode, with filter 2 feeding into filter 1.
         *
         * @param num_samples The number of samples in the current block.
         */
        void processSerialBackward(int num_samples);

        /**
         * @brief Initializes the FiltersModule, creating parameter controls and instantiating filter submodules.
         */
        void init() override;

        /**
         * @brief Clones the FiltersModule, creating a new instance with identical settings.
         *
         * @return A pointer to the newly cloned FiltersModule instance.
         */
        Processor* clone() const override { return new FiltersModule(*this); }

        /**
         * @brief Retrieves the on/off value control for filter 1, if it exists.
         * @return A pointer to the Value object controlling filter 1's on/off state.
         */
        const Value* getFilter1OnValue() const { return filter_1_->getOnValue(); }

        /**
         * @brief Retrieves the on/off value control for filter 2, if it exists.
         * @return A pointer to the Value object controlling filter 2's on/off state.
         */
        const Value* getFilter2OnValue() const { return filter_2_->getOnValue(); }

        /**
         * @brief Sets the oversampling amount and updates internal buffers accordingly.
         *
         * @param oversample The new oversampling factor.
         */
        void setOversampleAmount(int oversample) override {
            SynthModule::setOversampleAmount(oversample);
            filter_1_input_->ensureBufferSize(oversample * kMaxBufferSize);
            filter_2_input_->ensureBufferSize(oversample * kMaxBufferSize);
        }

    protected:
        FilterModule* filter_1_; ///< The first filter module instance.
        FilterModule* filter_2_; ///< The second filter module instance.

        Value* filter_1_filter_input_; ///< Determines if filter 1 is connected in series/parallel routing.
        Value* filter_2_filter_input_; ///< Determines if filter 2 is connected in series/parallel routing.

        std::shared_ptr<Output> filter_1_input_; ///< Internal output buffer feeding filter 1.
        std::shared_ptr<Output> filter_2_input_; ///< Internal output buffer feeding filter 2.

        JUCE_LEAK_DETECTOR(FiltersModule)
    };
} // namespace vital
