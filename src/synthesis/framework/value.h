/**
 * @file value.h
 * @brief Declares Value processors that output a constant value and can be dynamically set.
 *
 * The Value class provides a constant output signal that can be changed by triggering its input.
 * It supports both audio-rate and control-rate operation modes. When a trigger is received, the
 * output value is updated for all lanes of polyphony simultaneously.
 */

#pragma once

#include "processor.h"

namespace vital {

    /**
     * @class Value
     * @brief A Processor that maintains and outputs a constant poly_float value.
     *
     * The Value processor outputs a constant value (poly_float) for each sample. This value can
     * be updated at runtime through a trigger input (kSet), allowing parameter changes that affect
     * all samples processed after the trigger.
     */
    class Value : public Processor {
    public:
        enum {
            kSet,       ///< Index of the "set value" trigger input.
            kNumInputs  ///< Total number of inputs for this Processor.
        };

        /**
         * @brief Constructs a Value processor.
         * @param value The initial value to output.
         * @param control_rate True if operating at control rate (single-sample output), otherwise audio rate.
         */
        Value(poly_float value = 0.0f, bool control_rate = false);

        /**
         * @brief Creates a clone of this Value processor.
         * @return A pointer to a new cloned Processor instance.
         */
        virtual Processor* clone() const override { return new Value(*this); }

        /**
         * @brief Processes a block of samples, writing the constant value to the output buffer.
         * @param num_samples The number of samples to process.
         */
        virtual void process(int num_samples) override;

        /**
         * @brief Updates the oversampling amount and ensures the output buffer matches the stored value.
         * @param oversample The new oversampling amount.
         */
        virtual void setOversampleAmount(int oversample) override;

        /**
         * @brief Returns the current mono_float value of the first lane.
         * @return The scalar value of the first lane.
         */
        force_inline mono_float value() const { return value_[0]; }

        /**
         * @brief Sets the internal value to a new poly_float.
         * @param value The new poly_float value.
         */
        virtual void set(poly_float value);

    protected:
        poly_float value_; ///< The constant output value.

        JUCE_LEAK_DETECTOR(Value)
    };

    namespace cr {
        /**
         * @class Value
         * @brief A control-rate variant of the Value processor.
         *
         * This class functions similarly to the main Value processor but only updates a single sample
         * at a time, suitable for control-rate signals (e.g., modulation parameters).
         */
        class Value : public ::vital::Value {
        public:
            /**
             * @brief Constructs a control-rate Value processor.
             * @param value The initial value.
             */
            Value(poly_float value = 0.0f) : ::vital::Value(value, true) { }

            /**
             * @brief Creates a clone of this control-rate Value processor.
             * @return A pointer to a new cloned Processor instance.
             */
            virtual Processor* clone() const override { return new Value(*this); }

            /**
             * @brief Processes one "control-rate" sample, updating the output if triggered.
             * @param num_samples The number of samples to process (should be 1 for control-rate).
             */
            void process(int num_samples) override;
        };
    } // namespace cr
} // namespace vital
