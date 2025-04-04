/*
Summary:
The PitchDetector class attempts to find the fundamental period of a loaded signal using a method inspired by the YIN algorithm. By computing an error metric that measures how well the waveform repeats at different periods, it identifies a period length that best represents the fundamental frequency. This helps ensure that wavetables and other audio processing steps align wave cycles with their fundamental pitch.
 */

#pragma once

#include "JuceHeader.h"

/**
 * @brief A utility class for estimating the pitch (fundamental period) of a given audio signal segment.
 *
 * The PitchDetector class loads a segment of audio samples and attempts to determine its fundamental period
 * using a variation of the YIN pitch detection algorithm. By estimating the period length that minimizes
 * differences between segments of the waveform, it can guide other components (like wavetable extraction)
 * to align frames to a fundamental cycle.
 */
class PitchDetector {
public:
    /**
     * @brief A fixed number of points used in the period error computation.
     *
     * This number controls how many sample comparisons are made to estimate the signal's period.
     */
    static constexpr int kNumPoints = 2520;

    /**
     * @brief Constructs a PitchDetector with no loaded signal.
     */
    PitchDetector();

    /**
     * @brief Sets the internal size of the signal used for pitch detection.
     *
     * @param size Number of samples in the loaded signal.
     */
    void setSize(int size) { size_ = size; }

    /**
     * @brief Loads a signal into the PitchDetector for analysis.
     *
     * Copies the given signal into the detector's buffer.
     *
     * @param signal Pointer to the signal samples.
     * @param size Number of samples in the signal.
     */
    void loadSignal(const float* signal, int size);

    /**
     * @brief Computes the error metric for a given period length.
     *
     * A lower error typically indicates a more likely fundamental period. This function uses differences
     * between successive segments of the waveform to compute the error.
     *
     * @param period The candidate period length in samples.
     * @return An error metric for how well this period fits the waveform.
     */
    float getPeriodError(float period);

    /**
     * @brief Searches for a period using a YIN-like algorithm, up to a specified maximum period.
     *
     * Evaluates candidate periods and selects the one with minimal error.
     *
     * @param max_period The maximum period length to consider, in samples.
     * @return The detected period length in samples.
     */
    float findYinPeriod(int max_period);

    /**
     * @brief High-level method to find the best matching period using the YIN approach.
     *
     * @param max_period The maximum period length to consider.
     * @return The best matching period in samples.
     */
    float matchPeriod(int max_period);

    /**
     * @brief Returns a pointer to the internal signal data buffer.
     *
     * @return A const pointer to the loaded signal samples.
     */
    const float* data() const { return signal_data_.get(); }

protected:
    int size_;                                    ///< Number of samples in the loaded signal.
    std::unique_ptr<float[]> signal_data_;        ///< Buffer holding the loaded signal samples.

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(PitchDetector)
};
