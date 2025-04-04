/**
 * @file wave_frame_test.h
 * @brief Declares the WaveFrameTest class, which tests the WaveFrame class functionality.
 */

#pragma once

#include "JuceHeader.h"

/**
 * @class WaveFrameTest
 * @brief A test class to verify the correctness and stability of wave frame time-frequency conversions.
 *
 * The WaveFrameTest class tests the conversion of a waveform from the time domain to the frequency domain
 * and back, ensuring that the inverse transform closely reproduces the original waveform.
 */
class WaveFrameTest : public UnitTest {
public:
    /**
     * @brief Constructs a new WaveFrameTest with a specified test name and category.
     */
    WaveFrameTest() : UnitTest("Wave Frame", "Lookups") { }

    /**
     * @brief Runs all tests on the WaveFrame class.
     */
    void runTest() override;

    /**
     * @brief Tests random wave frame time-frequency conversion by comparing an original waveform to its
     * inverse-transformed result, ensuring minimal error.
     */
    void testRandomTimeFrequencyConversion();
};
