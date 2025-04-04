/**
 * @file legato_filter.h
 * @brief Defines the LegatoFilter class used to handle legato triggering behavior in a voice.
 */

#pragma once

#include "processor.h"

namespace vital {

    /**
     * @class LegatoFilter
     * @brief A processor that filters note triggers to implement legato behavior.
     *
     * The LegatoFilter processor ensures that retriggering only occurs when needed
     * based on legato mode and voice trigger states. If the voice should not retrigger
     * (due to legato being enabled and the previous note not being released), it will
     * block the trigger.
     */
    class LegatoFilter : public Processor {
    public:
        /**
         * @enum InputIndices
         * @brief Indices for the input ports.
         */
        enum {
            kLegato,  ///< Input that determines if legato is enabled.
            kTrigger, ///< Input trigger signal for the voice.
            kNumInputs
        };

        /**
         * @enum OutputIndices
         * @brief Indices for the output ports.
         */
        enum {
            kRetrigger, ///< Output trigger signal after legato filtering.
            kNumOutputs
        };

        /**
         * @brief Constructs a new LegatoFilter processor.
         */
        LegatoFilter();

        /**
         * @brief Creates a copy of this processor.
         * @return A pointer to a new LegatoFilter instance with the same configuration.
         */
        virtual Processor* clone() const override {
            return new LegatoFilter(*this);
        }

        /**
         * @brief Processes a block of samples and applies legato filtering to the trigger signals.
         * @param num_samples The number of samples to process.
         */
        void process(int num_samples) override;

    private:
        poly_float last_value_; ///< The last processed trigger value, used to determine retrigger behavior.

        JUCE_LEAK_DETECTOR(LegatoFilter)
    };
} // namespace vital
