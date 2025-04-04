/**
 * @file value_switch.cpp
 * @brief Implements the ValueSwitch class, allowing dynamic routing of one of multiple inputs.
 */

#include "value_switch.h"
#include "utils.h"

namespace vital {

    ValueSwitch::ValueSwitch(mono_float value) : cr::Value(value) {
        // Ensure both outputs (kValue, kSwitch) exist.
        while (numOutputs() < kNumOutputs)
            addOutput();

        // By default, the ValueSwitch is not enabled since it simply
        // passes through an input based on the set value.
        enable(false);
    }

    void ValueSwitch::set(poly_float value) {
        cr::Value::set(value);
        // Update the output source based on the first voice's integer value.
        setSource(value[0]);
    }

    void ValueSwitch::setOversampleAmount(int oversample) {
        cr::Value::setOversampleAmount(oversample);
        int num_inputs = numInputs();
        // Update oversampling for all inputs' owners.
        for (int i = 0; i < num_inputs; ++i) {
            input(i)->source->owner->setOversampleAmount(oversample);
        }
        // Reset the buffer to match the currently set value.
        setBuffer(value_[0]);
    }

    force_inline void ValueSwitch::setBuffer(int source) {
        // Clamp the source index to the available inputs.
        source = utils::iclamp(source, 0, numInputs() - 1);

        // Route the selected input buffer directly to the kSwitch output.
        output(kSwitch)->buffer = input(source)->source->buffer;
        output(kSwitch)->buffer_size = input(source)->source->buffer_size;
    }

    force_inline void ValueSwitch::setSource(int source) {
        // Set the new buffer based on the source index.
        setBuffer(source);

        // Enable or disable linked processors based on the chosen source.
        bool enable_processors = (source != 0);
        for (Processor* processor : processors_)
            processor->enable(enable_processors);
    }
} // namespace vital
