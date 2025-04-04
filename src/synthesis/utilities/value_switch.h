/**
 * @file value_switch.h
 * @brief Declares the ValueSwitch class, which allows switching the output buffer based on a control value.
 */

#pragma once

#include "value.h"

namespace vital {

    /**
     * @class ValueSwitch
     * @brief A specialized Value processor that selects one of multiple input sources to pass through,
     *        based on its own control value.
     *
     * The ValueSwitch processor reads a numeric value (an integer index) and uses it to select
     * which of its input buffers to output through the kSwitch output. When the value changes, the
     * selected input and associated processing is updated. This can be useful for conditional routing
     * of control signals.
     *
     * Inputs: None directly as typical Value inputs, but sources can be plugged in as inputs.
     *
     * Outputs:
     * - kValue: The control value itself (from the base Value class).
     * - kSwitch: The selected input's buffer.
     */
    class ValueSwitch : public cr::Value {
    public:
        /**
         * @enum OutputIndices
         * @brief Output buffer indices for this processor.
         */
        enum {
            kValue,   ///< The control value output (original from Value).
            kSwitch,  ///< The selected input signal output.
            kNumOutputs
        };

        /**
         * @brief Constructs a new ValueSwitch with an initial value.
         * @param value The initial control value to select an input.
         */
        ValueSwitch(mono_float value = 0.0f);

        /**
         * @brief Clones this ValueSwitch processor.
         * @return A pointer to a newly allocated ValueSwitch object that is a copy of this one.
         */
        virtual Processor* clone() const override { return new ValueSwitch(*this); }

        /**
         * @brief Processes the value switch. In this case, processing is not sample-based,
         *        so this function does nothing.
         * @param num_samples The number of samples to process.
         */
        virtual void process(int num_samples) override { }

        /**
         * @brief Sets the control value, selecting the corresponding input as the output.
         * @param value The new control value.
         */
        virtual void set(poly_float value) override;

        /**
         * @brief Adds a processor to be enabled or disabled depending on the selected source.
         * @param processor A pointer to a Processor to be toggled based on the current value.
         */
        void addProcessor(Processor* processor) { processors_.push_back(processor); }

        /**
         * @brief Sets the oversampling amount for this ValueSwitch and all connected sources.
         * @param oversample The new oversampling amount.
         */
        virtual void setOversampleAmount(int oversample) override;

    private:
        /**
         * @brief Sets the output buffer based on the given source index.
         * @param source The index of the source input to route.
         */
        void setBuffer(int source);

        /**
         * @brief Selects the source based on the given integer value and enables or disables linked processors.
         * @param source The source index.
         */
        void setSource(int source);

        /// Processors that are conditionally enabled or disabled based on the selected source.
        std::vector<Processor*> processors_;

        JUCE_LEAK_DETECTOR(ValueSwitch)
    };
} // namespace vital
