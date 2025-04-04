/**
 * @file synth_lfo_test.h
 * @brief Declares the SynthLfoTest class, which tests the SynthLfo processor.
 */

#pragma once

#include "processor_test.h"

/**
 * @class SynthLfoTest
 * @brief A test class that verifies the stability and correctness of the SynthLfo processor.
 *
 * This test checks whether the SynthLfo processor, which uses a LineGenerator as a source,
 * can handle extreme input conditions without producing non-finite outputs. By leveraging the
 * ProcessorTest framework, it applies standardized input bounds tests while ignoring certain
 * irrelevant outputs.
 */
class SynthLfoTest : public ProcessorTest {
public:
    /**
     * @brief Constructs a new SynthLfoTest with the specified test name.
     */
    SynthLfoTest() : ProcessorTest("Synth Lfo") { }

    /**
     * @brief Runs the SynthLfo test by performing input bounds checks. Certain outputs, such as OscPhase,
     *        are ignored in this test.
     */
    void runTest() override;
};
