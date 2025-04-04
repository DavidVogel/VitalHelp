#include "line_map.h"

#include "line_generator.h"
#include "utils.h"

namespace vital {

    LineMap::LineMap(LineGenerator* source) : Processor(1, kNumOutputs, true), source_(source) { }

    void LineMap::process(int /*num_samples*/) {
        /**
         * @brief High-level process call that retrieves the input phase and delegates to `process(poly_float phase)`.
         *
         * The input phase is taken from the first input's buffer at index 0. The `num_samples` parameter is
         * not directly used since this processor works on a per-sample (or per-block) basis.
         */
        process(input()->at(0));
    }

    void LineMap::process(poly_float phase) {
        /**
         * @brief Interpolates a value from the line generator's data buffer based on the input phase.
         *
         * The phase is scaled by the resolution of the line generator and then used to select the
         * appropriate segment of the line. Cubic interpolation is performed to achieve smooth transitions between points.
         * The result is clamped between -1.0f and 1.0f and written to the output buffer.
         */

        // Retrieve the line generator's cubic interpolation buffer and resolution.
        mono_float* buffer = source_->getCubicInterpolationBuffer();
        int resolution = source_->resolution();

        // Convert the phase into an index into the buffer.
        poly_float boost = utils::clamp(phase * resolution, 0.0f, (mono_float)resolution);
        poly_int indices = utils::clamp(utils::toInt(boost), 0, resolution - 1);
        poly_float t = boost - utils::toFloat(indices);

        // Compute interpolation matrices and apply cubic interpolation.
        matrix interpolation_matrix = utils::getPolynomialInterpolationMatrix(t);
        matrix value_matrix = utils::getValueMatrix(buffer, indices);

        // Transpose the value matrix to align dimensions for multiplication.
        value_matrix.transpose();

        // Perform matrix multiplication to get the interpolated result.
        poly_float result = utils::clamp(interpolation_matrix.multiplyAndSumRows(value_matrix), -1.0f, 1.0f);

        // Output the computed value and the original phase.
        output(kValue)->buffer[0] = result;
        output(kPhase)->buffer[0] = phase;
    }
} // namespace vital
