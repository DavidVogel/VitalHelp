/**
 * @file wave_frame_test.cpp
 * @brief Implements the WaveFrameTest class, running tests on the WaveFrame class time-frequency conversion.
 */

#include "wave_frame_test.h"
#include "wave_frame.h"

void WaveFrameTest::runTest() {
    testRandomTimeFrequencyConversion();
}

void WaveFrameTest::testRandomTimeFrequencyConversion() {
    static constexpr float kMaxError = 0.00001f;

    beginTest("Test Random Wave Frame Time Frequency Conversion");

    vital::WaveFrame wave_frame;
    vital::mono_float original[vital::WaveFrame::kWaveformSize];

    // Generate a random waveform in the time domain
    for (int i = 0; i < vital::WaveFrame::kWaveformSize; ++i) {
        original[i] = (2.0f * rand()) / RAND_MAX - 1.0f; // random values in [-1, 1]
        wave_frame.time_domain[i] = original[i];
    }

    // Convert to frequency domain and back to time domain
    wave_frame.toFrequencyDomain();
    wave_frame.toTimeDomain();

    // Check that the reconstructed waveform matches the original within a small error tolerance
    for (int i = 0; i < vital::WaveFrame::kWaveformSize; ++i) {
        float error = wave_frame.time_domain[i] - original[i];
        expect(std::abs(error) < kMaxError, "Fourier Inverse gave big error.");
    }
}

// Register the test so it will be automatically discovered and run.
static WaveFrameTest wave_frame_test;
