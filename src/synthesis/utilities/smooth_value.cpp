/**
 * @file smooth_value.cpp
 * @brief Implements the SmoothValue classes, performing exponential and optional linear smoothing of values.
 */

#include "smooth_value.h"
#include <cmath>
#include "futils.h"

namespace vital {

    SmoothValue::SmoothValue(mono_float value) : Value(value), current_value_(value) { }

    void SmoothValue::process(int num_samples) {
        // If all values are equal (current_value, target value, and output), no need to continue processing.
        if (utils::equal(current_value_, value_) && utils::equal(current_value_, output()->buffer[0]) &&
            utils::equal(current_value_, output()->buffer[num_samples - 1])) {
            enable(false);
            return;
        }

        // Calculate the exponential decay factor based on the smoothing cutoff and sample rate.
        mono_float decay = futils::exp(-2.0f * kPi * kSmoothCutoff / getSampleRate());
        poly_float current_value = current_value_;
        poly_float target_value = value_;
        poly_float* dest = output()->buffer;

        // Apply exponential smoothing sample-by-sample.
        for (int i = 0; i < num_samples; ++i) {
            current_value = utils::interpolate(target_value, current_value, decay);
            dest[i] = current_value;
        }

        // Check if conditions are met for linear interpolation for a final smooth step.
        poly_mask equal_mask = poly_float::equal(current_value, current_value_) |
                               poly_float::equal(value_, current_value_);
        if (equal_mask.anyMask())
            linearInterpolate(num_samples, equal_mask);

        current_value_ = utils::maskLoad(current_value, current_value_, equal_mask);
    }

    void SmoothValue::linearInterpolate(int num_samples, poly_mask linear_mask) {
        // If linear interpolation is needed, we linearly ramp the value.
        poly_float current_value = current_value_;
        current_value_ = utils::maskLoad(current_value_, value_, linear_mask);
        poly_float delta_value = (value_ - current_value) * (1.0f / num_samples);

        poly_float* dest = output()->buffer;
        for (int i = 0; i < num_samples; ++i) {
            current_value += delta_value;
            dest[i] = utils::maskLoad(dest[i], current_value, linear_mask);
        }

        // Fill remaining buffer with the final value if needed.
        int max_samples = output()->buffer_size;
        for (int i = num_samples; i < max_samples; ++i)
            dest[i] = current_value_;
    }

    namespace cr {
        SmoothValue::SmoothValue(mono_float value) : Value(value), current_value_(value) { }

        void SmoothValue::process(int num_samples) {
            // Compute decay based on control-rate updates.
            mono_float decay = futils::exp(-2.0f * kPi * kSmoothCutoff * num_samples / getSampleRate());
            current_value_ = utils::interpolate(value_, current_value_, decay);
            output()->buffer[0] = current_value_;
        }
    } // namespace cr
} // namespace vital
