/**
 * @file legato_filter.cpp
 * @brief Implements the LegatoFilter class methods for legato-based trigger filtering.
 */

#include "legato_filter.h"
#include "utils.h"

namespace vital {

    LegatoFilter::LegatoFilter()
            : Processor(kNumInputs, kNumOutputs, true),
              last_value_(kVoiceOff) {
    }

    void LegatoFilter::process(int num_samples) {
        // Clear any previous triggers on the output before processing.
        output(kRetrigger)->clearTrigger();

        // Get the current trigger mask from the input trigger source.
        poly_mask trigger_mask = input(kTrigger)->source->trigger_mask;
        if (trigger_mask.anyMask() == 0)
            return; // No triggers to process, so return early.

        // Retrieve the trigger value and offset from the input.
        poly_float trigger_value = input(kTrigger)->source->trigger_value;
        poly_int trigger_offset = input(kTrigger)->source->trigger_offset;

        // Determine which voices should be retriggered considering the legato state.
        // legato_mask will be true when we need to block retriggers due to legato rules.
        poly_mask legato_mask = poly_float::equal(input(kLegato)->at(0), 0.0f);
        legato_mask |= poly_float::notEqual(trigger_value, kVoiceOn);
        legato_mask |= poly_float::notEqual(last_value_, kVoiceOn);
        trigger_mask &= legato_mask;

        // Trigger the output when conditions are met.
        output(kRetrigger)->trigger(trigger_mask, trigger_value, trigger_offset);

        // Update the last_value_ based on which voices triggered.
        last_value_ = utils::maskLoad(last_value_, trigger_value, trigger_mask);
    }
} // namespace vital
