#include "trigger_random.h"

#include <cstdlib>

namespace vital {

    TriggerRandom::TriggerRandom() :
            Processor(1, 1, true), // 1 input (trigger), 1 output, control-rate capable
            value_(0.0f),
            random_generator_(0.0f, 1.0f) { }

    void TriggerRandom::process(int /*num_samples*/) {
        /**
         * @brief Checks for a trigger event (reset) and, if found, generates a new random value.
         *
         * If the reset input is triggered, a new random value is assigned to both elements of a stereo pair
         * (or multiple voices if applicable). This value remains constant until the next trigger event.
         */
        poly_mask trigger_mask = getResetMask(kReset);
        if (trigger_mask.anyMask()) {
            // Iterate through voices in pairs (assuming poly_float pack size)
            for (int i = 0; i < poly_float::kSize; i += 2) {
                // Check if this voice is triggered
                if ((poly_float(1.0f) & trigger_mask)[i]) {
                    mono_float rand_value = random_generator_.next();
                    value_.set(i, rand_value);
                    value_.set(i + 1, rand_value);
                }
            }
        }

        output()->buffer[0] = value_;
    }
} // namespace vital
