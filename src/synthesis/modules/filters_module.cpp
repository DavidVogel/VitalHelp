#include "filters_module.h"
#include "filter_module.h"

namespace vital {

    FiltersModule::FiltersModule() :
            SynthModule(kNumInputs, 1),
            filter_1_(nullptr), filter_2_(nullptr),
            filter_1_filter_input_(nullptr), filter_2_filter_input_(nullptr) {
        // Allocate shared outputs for feeding each filter.
        filter_1_input_ = std::make_shared<Output>();
        filter_2_input_ = std::make_shared<Output>();
    }

    void FiltersModule::init() {
        /**
         * @brief Initializes the FiltersModule by creating two FilterModule instances and connecting inputs/outputs.
         *
         * This sets up parameters for choosing the routing mode and connecting the audio inputs to the filters.
         * filter_1_filter_input_ and filter_2_filter_input_ control how the filters are chained or combined.
         */

        filter_1_filter_input_ = createBaseControl("filter_1_filter_input");
        filter_1_ = new FilterModule("filter_1");
        addSubmodule(filter_1_);
        addProcessor(filter_1_);

        filter_1_->plug(filter_1_input_.get(), FilterModule::kAudio);
        filter_1_->useInput(input(kReset), FilterModule::kReset);
        filter_1_->useInput(input(kKeytrack), FilterModule::kKeytrack);
        filter_1_->useInput(input(kMidi), FilterModule::kMidi);

        filter_2_filter_input_ = createBaseControl("filter_2_filter_input");
        filter_2_ = new FilterModule("filter_2");
        addSubmodule(filter_2_);
        addProcessor(filter_2_);

        filter_2_->plug(filter_2_input_.get(), FilterModule::kAudio);
        filter_2_->useInput(input(kReset), FilterModule::kReset);
        filter_2_->useInput(input(kKeytrack), FilterModule::kKeytrack);
        filter_2_->useInput(input(kMidi), FilterModule::kMidi);

        SynthModule::init();
    }

    void FiltersModule::processParallel(int num_samples) {
        /**
         * @brief Processes both filters in parallel and sums their outputs.
         *
         * filter_1_input_ and filter_2_input_ are set from separate inputs.
         * Each filter is processed independently, and their outputs are mixed together.
         */
        filter_1_input_->buffer = input(kFilter1Input)->source->buffer;
        filter_2_input_->buffer = input(kFilter2Input)->source->buffer;

        getLocalProcessor(filter_1_)->process(num_samples);
        getLocalProcessor(filter_2_)->process(num_samples);

        poly_float* output_buffer = output()->buffer;
        const poly_float* filter_1_buffer = filter_1_->output()->buffer;
        const poly_float* filter_2_buffer = filter_2_->output()->buffer;

        for (int i = 0; i < num_samples; ++i)
            output_buffer[i] = filter_1_buffer[i] + filter_2_buffer[i];
    }

    void FiltersModule::processSerialForward(int num_samples) {
        /**
         * @brief Processes filters in serial (forward) mode.
         *
         * Filter 1 processes input(kFilter1Input), then its output is combined with input(kFilter2Input) and fed into filter 2.
         * The final output is the result of filter 2.
         */
        filter_1_input_->buffer = input(kFilter1Input)->source->buffer;
        filter_2_input_->buffer = filter_2_input_->owned_buffer.get();

        getLocalProcessor(filter_1_)->process(num_samples);

        poly_float* filter_2_input_buffer = filter_2_input_->buffer;
        const poly_float* filter_1_output_buffer = filter_1_->output()->buffer;
        const poly_float* filter_2_straight_input = input(kFilter2Input)->source->buffer;

        for (int i = 0; i < num_samples; ++i)
            filter_2_input_buffer[i] = filter_1_output_buffer[i] + filter_2_straight_input[i];

        getLocalProcessor(filter_2_)->process(num_samples);
        utils::copyBuffer(output()->buffer, filter_2_->output()->buffer, num_samples);
    }

    void FiltersModule::processSerialBackward(int num_samples) {
        /**
         * @brief Processes filters in serial (backward) mode.
         *
         * Filter 2 processes input(kFilter2Input), then its output is combined with input(kFilter1Input) and fed into filter 1.
         * The final output is the result of filter 1.
         */
        filter_1_input_->buffer = filter_1_input_->owned_buffer.get();
        filter_2_input_->buffer = input(kFilter2Input)->source->buffer;

        getLocalProcessor(filter_2_)->process(num_samples);

        poly_float* filter_1_input_buffer = filter_1_input_->buffer;
        const poly_float* filter_2_output_buffer = filter_2_->output()->buffer;
        const poly_float* filter_1_straight_input = input(kFilter1Input)->source->buffer;

        for (int i = 0; i < num_samples; ++i)
            filter_1_input_buffer[i] = filter_2_output_buffer[i] + filter_1_straight_input[i];

        getLocalProcessor(filter_1_)->process(num_samples);
        utils::copyBuffer(output()->buffer, filter_1_->output()->buffer, num_samples);
    }

    void FiltersModule::process(int num_samples) {
        /**
         * @brief Chooses the appropriate routing mode (parallel, serial forward, or serial backward) based on control parameters.
         *
         * If filter_1_filter_input_ and filter_1_->getOnValue() are active, it suggests serial backward mode.
         * If filter_2_filter_input_ and filter_2_->getOnValue() are active, it suggests serial forward mode.
         * Otherwise, the filters are processed in parallel.
         */
        if (filter_1_filter_input_->value() && filter_1_->getOnValue()->value())
            processSerialBackward(num_samples);
        else if (filter_2_filter_input_->value() && filter_2_->getOnValue()->value())
            processSerialForward(num_samples);
        else
            processParallel(num_samples);
    }
} // namespace vital
