#pragma once

#include "common.h"
#include "synth_constants.h"
#include "utils.h"

namespace vital {

    /**
     * @class OnePoleFilter
     * @brief A one-pole filter implementation with optional nonlinear saturation.
     *
     * This template class implements a one-pole filter with support for custom saturation functions.
     * It can be used as a basic low-pass filter or as part of more complex filter structures, such as
     * ladder or diode filters. By default, it does not apply saturation (using @c utils::pass), but
     * a saturation function can be provided to add nonlinear character to the filter.
     *
     * @tparam saturate A function pointer to a saturation function that takes a poly_float and returns a poly_float.
     *                  By default, it is @c utils::pass, which applies no saturation.
     */
    template <poly_float (*saturate)(poly_float) = utils::pass<poly_float>>
    class OnePoleFilter {
    public:
        /**
         * @brief Constructs a OnePoleFilter and resets its internal state.
         */
        OnePoleFilter() {
            reset(constants::kFullMask);
        }

        /**
         * @brief Resets the filter state for the voices indicated by a mask.
         *
         * @param reset_mask A mask specifying which voices to reset.
         */
        force_inline void reset(poly_mask reset_mask) {
            current_state_ = utils::maskLoad(current_state_, 0.0f, reset_mask);
            filter_state_ = utils::maskLoad(filter_state_, 0.0f, reset_mask);
            sat_filter_state_ = utils::maskLoad(sat_filter_state_, 0.0f, reset_mask);
        }

        /**
         * @brief Destructor.
         */
        virtual ~OnePoleFilter() { }

        /**
         * @brief Processes a single sample in a basic (non-saturating) manner.
         *
         * This method applies a one-pole low-pass operation without using the saturation function.
         *
         * @param audio_in The input sample to process.
         * @param coefficient The filter coefficient (determined by cutoff frequency and sample rate).
         * @return The filtered output sample.
         */
        force_inline poly_float tickBasic(poly_float audio_in, poly_float coefficient) {
            poly_float delta = coefficient * (audio_in - filter_state_);
            filter_state_ += delta;
            current_state_ = filter_state_;
            filter_state_ += delta;
            return current_state_;
        }

        /**
         * @brief Processes a single sample, applying the saturation function at each step.
         *
         * This method is used when nonlinear saturation is desired.
         *
         * @param audio_in The input sample to process.
         * @param coefficient The filter coefficient.
         * @return The filtered output sample with saturation applied.
         */
        force_inline poly_float tick(poly_float audio_in, poly_float coefficient) {
            poly_float delta = coefficient * (audio_in - sat_filter_state_);
            filter_state_ += delta;
            current_state_ = saturate(filter_state_);
            filter_state_ += delta;
            sat_filter_state_ = saturate(filter_state_);
            return current_state_;
        }

        /**
         * @brief Processes a single sample using a derivative form that includes saturation.
         *
         * This method uses a more complex update step that might be used for certain filter types
         * where a derivative-based update is needed.
         *
         * @param audio_in The input sample to process.
         * @param coefficient The filter coefficient.
         * @return The filtered output sample with saturation and derivative-based updates applied.
         */
        force_inline poly_float tickDerivative(poly_float audio_in, poly_float coefficient) {
            poly_float delta = coefficient * (audio_in - filter_state_);
            filter_state_ = utils::mulAdd(filter_state_, saturate(filter_state_ + delta), delta);
            current_state_ = filter_state_;
            filter_state_ = utils::mulAdd(filter_state_, saturate(filter_state_ + delta), delta);
            sat_filter_state_ = filter_state_;
            return current_state_;
        }

        /**
         * @brief Gets the current state of the filter output.
         *
         * @return The current output sample of the filter.
         */
        force_inline poly_float getCurrentState() { return current_state_; }

        /**
         * @brief Gets the next saturated filter state value.
         *
         * @return The state of the filter after saturation is applied, useful for feedback calculations.
         */
        force_inline poly_float getNextSatState() { return sat_filter_state_; }

        /**
         * @brief Gets the next filter state value (without saturation).
         *
         * @return The next filter state, useful for internal computations.
         */
        force_inline poly_float getNextState() { return filter_state_; }

        /**
         * @brief Computes the filter coefficient for a given cutoff frequency and sample rate.
         *
         * This method is static and can be used to determine the coefficient before processing samples.
         *
         * @param cutoff_frequency The desired cutoff frequency in Hz.
         * @param sample_rate The current sample rate in Hz.
         * @return The computed filter coefficient.
         */
        static force_inline poly_float computeCoefficient(poly_float cutoff_frequency, int sample_rate) {
            poly_float delta_phase = cutoff_frequency * (vital::kPi / sample_rate);
            return utils::tan(delta_phase / (delta_phase + 1.0f));
        }

    private:
        poly_float current_state_;     ///< The current output state of the filter.
        poly_float filter_state_;      ///< The internal filter state before saturation.
        poly_float sat_filter_state_;  ///< The internal filter state after saturation.
    };
} // namespace vital
