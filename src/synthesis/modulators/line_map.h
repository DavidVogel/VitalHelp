#pragma once

#include "processor.h"

class LineGenerator;

namespace vital {

    /**
     * @brief A processor that maps a phase input through a line generator, producing a value and phase output.
     *
     * The LineMap class is designed to take a phase input (e.g., from an oscillator or another modulation source)
     * and use it to index into a line generator's data. It uses cubic interpolation to produce a continuous
     * output value corresponding to the given phase. The result is clamped to a specified range to ensure output stability.
     */
    class LineMap : public Processor {
    public:
        /// The maximum allowable curvature/power for certain operations within the line mapping.
        static constexpr mono_float kMaxPower = 20.0f;

        /**
         * @brief Indices of the outputs produced by LineMap.
         *
         * - kValue: The interpolated value derived from the line generator for the given phase.
         * - kPhase: The input phase value, passed through to the output for reference.
         */
        enum MapOutput {
            kValue,
            kPhase,
            kNumOutputs
        };

        /**
         * @brief Constructs a LineMap processor.
         *
         * @param source Pointer to the LineGenerator object this LineMap will use for data lookup.
         */
        LineMap(LineGenerator* source);

        /**
         * @brief Creates a duplicate of this LineMap processor.
         *
         * @return A pointer to the newly cloned LineMap instance.
         */
        virtual Processor* clone() const override { return new LineMap(*this); }

        /**
         * @brief Processes a given number of samples at the current rate (audio or control).
         *
         * This reads the input phase from the first input and calls the other `process` method
         * to handle the interpolation and output generation.
         *
         * @param num_samples Number of samples to process (unused directly, as we process a single value here).
         */
        void process(int num_samples) override;

        /**
         * @brief Processes a given phase value by interpolating from the line generator's data.
         *
         * @param phase The input phase (0 to 1) that indexes into the line generator's precomputed line.
         */
        void process(poly_float phase);

    protected:
        poly_float offset_;       ///< An offset that could be applied to the output (not currently set in this code).
        LineGenerator* source_;   ///< The associated LineGenerator used for generating the values.

        JUCE_LEAK_DETECTOR(LineMap)
    };
} // namespace vital
