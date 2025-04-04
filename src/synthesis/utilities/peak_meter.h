/**
 * @file peak_meter.h
 * @brief Defines the PeakMeter class, a processor that measures and reports peak and memory-peak levels of audio signals.
 */

#pragma once

#include "processor.h"

namespace vital {

    /**
     * @class PeakMeter
     * @brief A processor that computes both instantaneous peak and a "memory peak" of an incoming audio signal.
     *
     * The PeakMeter measures the level of the audio signal and also keeps track of a remembered peak
     * level over a short period. This can be used to display peak hold meters, for example,
     * where the highest recent level is displayed momentarily before decaying.
     *
     * Inputs:
     * - 0: The audio input signal.
     *
     * Outputs:
     * - kLevel: The current peak (or processed level) of the input signal.
     * - kMemoryPeak: The highest (remembered) peak recently detected.
     */
    class PeakMeter : public Processor {
    public:
        /// Maximum number of remembered peaks to consider.
        static constexpr int kMaxRememberedPeaks = 16;

        /**
         * @enum OutputIndices
         * @brief Indices for the output buffers.
         */
        enum {
            kLevel,       ///< Current peak output index.
            kMemoryPeak,  ///< Memory-peak output index.
            kNumOutputs   ///< Number of outputs.
        };

        /**
         * @brief Constructs a new PeakMeter processor.
         */
        PeakMeter();

        /**
         * @brief Creates a clone of this PeakMeter processor.
         * @return A pointer to a newly allocated PeakMeter object that is a copy of this one.
         */
        virtual Processor* clone() const override { return new PeakMeter(*this); }

        /**
         * @brief Processes the input audio to compute current and remembered peak levels.
         *
         * @param num_samples The number of audio samples to process.
         */
        void process(int num_samples) override;

    protected:
        /// Current instantaneous peak value.
        poly_float current_peak_;

        /// Sum of squared samples used for RMS/level calculations.
        poly_float current_square_sum_;

        /// The highest remembered peak level over a certain period.
        poly_float remembered_peak_;

        /// The number of samples since the remembered peak was updated.
        poly_int samples_since_remembered_;

        JUCE_LEAK_DETECTOR(PeakMeter)
    };
} // namespace vital
