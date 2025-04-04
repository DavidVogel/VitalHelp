/**
 * @file synth_oscillator_test.h
 * @brief Declares the SynthOscillatorTest class, which tests the SynthOscillator processor.
 */

#pragma once

#include "processor_test.h"

/**
 * @class SynthOscillatorTest
 * @brief A test class that, once fully implemented, will verify the stability and correctness of the SynthOscillator processor.
 *
 * Currently, the input bounds test call is commented out. Once the SynthOscillator is ready and
 * the expected behavior is defined, the input bounds tests and other relevant checks should be enabled.
 */
class SynthOscillatorTest : public ProcessorTest {
public:
    /**
     * @brief Constructs a new SynthOscillatorTest with the specified test name.
     */
    SynthOscillatorTest() : ProcessorTest("Synth Oscillator") { }

    /**
     * @brief Runs the SynthOscillator test.
     *
     * @note The actual input bounds test is commented out for now. Once the oscillator is ready for testing,
     *       uncomment and/or add the relevant testing code.
     */
    void runTest() override;
};
