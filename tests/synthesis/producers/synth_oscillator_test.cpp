/**
 * @file synth_oscillator_test.cpp
 * @brief Implements the SynthOscillatorTest class.
 *
 * This test is currently a placeholder. Once the SynthOscillator is fully integrated and its
 * behavior is well-defined, you can uncomment the input bounds test and perform additional checks.
 */

#include "synth_oscillator_test.h"
#include "synth_oscillator.h"
#include "wavetable.h"

void SynthOscillatorTest::runTest() {
    // Create a Wavetable for the oscillator.
    vital::Wavetable wavetable(vital::kNumOscillatorWaveFrames);

    // Create a SynthOscillator with the provided wavetable.
    std::unique_ptr<vital::SynthOscillator> osc = std::make_unique<vital::SynthOscillator>(&wavetable);

    // TODO: Uncomment and run the input bounds test once the oscillator is ready for testing.
    // runInputBoundsTest(osc.get());
}

// Registers the test instance so it will be automatically discovered and run.
// Currently, this test does nothing but can be expanded in the future.
static SynthOscillatorTest synth_oscillator_test;
