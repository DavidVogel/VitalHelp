/**
 * @file smooth_value.h
 * @brief Declares the SmoothValue classes, providing time-smoothed transitions for values.
 */

#pragma once

#include "value.h"

namespace vital {

    /**
     * @class SmoothValue
     * @brief A Value processor that smoothly transitions from its current value to a target value.
     *
     * This class provides a smoothed approach to changing values over time, useful for avoiding
     * clicks or sudden parameter changes. It applies an exponential decay-based smoothing.
     *
     * Inputs: None (controlled via set methods)
     * Outputs:
     * - A smoothed value that transitions gradually toward a target.
     */
    class SmoothValue : public Value {
    public:
        /// The cutoff frequency for smoothing, controlling how fast the value settles.
        static constexpr mono_float kSmoothCutoff = 5.0f;

        /**
         * @brief Constructs a new SmoothValue with an initial value.
         * @param value The initial value.
         */
        SmoothValue(mono_float value = 0.0f);

        /**
         * @brief Creates a clone of this SmoothValue processor.
         * @return A pointer to a newly allocated SmoothValue object that is a copy of this one.
         */
        virtual Processor* clone() const override {
            return new SmoothValue(*this);
        }

        /**
         * @brief Processes a block of samples, updating the smoothed value output.
         * @param num_samples The number of samples to process.
         */
        virtual void process(int num_samples) override;

        /**
         * @brief Performs a linear interpolation if conditions are met for a smoother transition.
         * @param num_samples The number of samples to process in the linear interpolation.
         * @param linear_mask A mask indicating which voices should use linear interpolation.
         */
        void linearInterpolate(int num_samples, poly_mask linear_mask);

        /**
         * @brief Sets the new target value, enabling the processor and starting the smoothing process.
         * @param value The new target value to smoothly transition toward.
         */
        void set(poly_float value) override {
            enable(true);
            value_ = value;
        }

        /**
         * @brief Immediately sets the value without smoothing, and updates internal state.
         * @param value The value to set immediately.
         */
        void setHard(poly_float value) {
            enable(true);
            Value::set(value);
            current_value_ = value;
        }

    private:
        /// The current smoothed value at the end of the last processing block.
        poly_float current_value_;
    };

    namespace cr {
        /**
         * @class SmoothValue
         * @brief A control-rate version of the SmoothValue that smooths values at control rate instead of audio rate.
         *
         * This variation is used for control-rate signals, typically lower frequency updates,
         * and uses a different smoothing cutoff suitable for slower updates.
         *
         * Inputs: None (controlled via set methods)
         * Outputs:
         * - A smoothed control-rate value.
         */
        class SmoothValue : public Value {
        public:
            /// The cutoff frequency for smoothing at control rate.
            static constexpr mono_float kSmoothCutoff = 20.0f;

            /**
             * @brief Constructs a new SmoothValue (control-rate) with an initial value.
             * @param value The initial value.
             */
            SmoothValue(mono_float value = 0.0f);

            /**
             * @brief Creates a clone of this SmoothValue (control-rate) processor.
             * @return A pointer to a newly allocated SmoothValue object that is a copy of this one.
             */
            virtual Processor* clone() const override {
                return new SmoothValue(*this);
            }

            /**
             * @brief Processes the control-rate smoothing for the given number of samples.
             *
             * @param num_samples The number of samples to process. Typically, this will be control-rate blocks.
             */
            virtual void process(int num_samples) override;

            /**
             * @brief Immediately sets the control-rate value without smoothing, and updates internal state.
             * @param value The value to set immediately.
             */
            void setHard(mono_float value) {
                Value::set(value);
                current_value_ = value;
            }

        private:
            /// The current smoothed control-rate value at the end of the last processing block.
            poly_float current_value_;

            JUCE_LEAK_DETECTOR(SmoothValue)
        };
    } // namespace cr
} // namespace vital
