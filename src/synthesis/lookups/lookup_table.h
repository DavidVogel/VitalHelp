/**
 * @file lookup_table.h
 * @brief Declares a templated one-dimensional lookup table for fast function evaluation.
 *
 * This header defines a templated class that uses a precomputed lookup table to approximate
 * a given function. The lookup table can be accessed with cubic interpolation to provide
 * smooth and efficient evaluations.
 */

#pragma once

#include "common.h"
#include "poly_utils.h"

namespace vital {

    /**
     * @class OneDimLookup
     * @brief A one-dimensional lookup table for a given function with a specified resolution.
     *
     * The OneDimLookup class precomputes values of a given function at a certain resolution.
     * It then provides a method to retrieve interpolated values using cubic interpolation, allowing
     * for fast, smooth approximations of the function.
     *
     * @tparam function A pointer to a function taking a mono_float and returning a mono_float.
     *                  This function will be sampled to build the lookup table.
     * @tparam resolution The number of sample points used to build the lookup table. Higher values
     *                    provide more accuracy but use more memory.
     */
    template<mono_float(*function)(mono_float), size_t resolution>
    class OneDimLookup {
        static constexpr int kExtraValues = 4; ///< Extra values for safe interpolation at boundaries.
    public:
        /**
         * @brief Constructs the lookup table by sampling the given function.
         * @param scale A scaling factor applied to the function's input before sampling.
         */
        OneDimLookup(float scale = 1.0f) {
            scale_ = resolution / scale;
            for (int i = 0; i < static_cast<int>(resolution) + kExtraValues; ++i) {
                mono_float t = (i - 1.0f) / (resolution - 1.0f);
                lookup_[i] = function(t * scale);
            }
        }

        /**
         * @brief Destructor.
         */
        ~OneDimLookup() { }

        /**
         * @brief Performs a cubic interpolation lookup on the precomputed data.
         *
         * This function takes a poly_float of input values, scales them, and uses cubic
         * interpolation to estimate the function's value at these points. It returns a poly_float
         * with the interpolated results.
         *
         * @param value The input poly_float values to lookup.
         * @return A poly_float containing the interpolated function values.
         */
        force_inline poly_float cubicLookup(poly_float value) const {
            poly_float boost = value * scale_;
            poly_int indices = utils::clamp(utils::toInt(boost), 0, static_cast<int>(resolution));
            poly_float t = boost - utils::toFloat(indices);

            matrix interpolation_matrix = utils::getCatmullInterpolationMatrix(t);
            matrix value_matrix = utils::getValueMatrix(lookup_, indices);
            value_matrix.transpose();

            return interpolation_matrix.multiplyAndSumRows(value_matrix);
        }

    private:
        mono_float lookup_[resolution + kExtraValues]; ///< The array holding the sampled function values.
        mono_float scale_;                             ///< Scaling factor relating input range to lookup indices.

        JUCE_LEAK_DETECTOR(OneDimLookup)
    };

} // namespace vital
