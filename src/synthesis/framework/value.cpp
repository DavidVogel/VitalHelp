/**
 * @file value.cpp
 * @brief Implements the Value classes that output a constant value and can update it via triggers.
 */

#include "value.h"
#include "utils.h"

namespace vital {

    Value::Value(poly_float value, bool control_rate) : Processor(kNumInputs, 1, control_rate), value_(value) {
        // Initialize output buffer with the initial value.
        for (int i = 0; i < output()->buffer_size; ++i)
            output()->buffer[i] = value_;
    }

    void Value::set(poly_float value) {
        // Update the internal value and refill the output buffer.
        value_ = value;
        for (int i = 0; i < output()->buffer_size; ++i)
            output()->buffer[i] = value;
    }

    void Value::process(int num_samples) {
        // Check for trigger to update the value.
        poly_mask trigger_mask = input(kSet)->source->trigger_mask;
        if (trigger_mask.anyMask()) {
            poly_float trigger_value = input(kSet)->source->trigger_value;
            value_ = utils::maskLoad(value_, trigger_value, trigger_mask);
        }

        // Write the current value to all samples.
        poly_float* dest = output()->buffer;
        for (int i = 0; i < num_samples; ++i)
            dest[i] = value_;
    }

    void Value::setOversampleAmount(int oversample) {
        Processor::setOversampleAmount(oversample);
        // After changing oversampling, ensure output is still correct.
        for (int i = 0; i < output()->buffer_size; ++i)
            output()->buffer[i] = value_;
    }

    void cr::Value::process(int num_samples) {
        // At control rate, typically num_samples = 1.
        poly_mask trigger_mask = input(kSet)->source->trigger_mask;
        if (trigger_mask.anyMask()) {
            poly_float trigger_value = input(kSet)->source->trigger_value;
            value_ = utils::maskLoad(value_, trigger_value, trigger_mask);
        }

        // Only one sample at control rate.
        output()->buffer[0] = value_;
    }
} // namespace vital
