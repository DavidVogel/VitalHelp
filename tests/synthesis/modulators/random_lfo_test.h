/**
 * @file random_lfo_test.h
 * @brief Declares the RandomLfoTest class, which tests the RandomLfo processor.
 */

#pragma once

#include "processor_test.h"

/**
 * @class RandomLfoTest
 * @brief A test class that verifies the stability and correctness of the RandomLfo processor.
 *
 * This test checks whether the RandomLfo processor handles extreme input conditions without producing
 * invalid (non-finite) outputs. By leveraging the ProcessorTest framework, it applies standardized input
 * bounds tests to ensure the RandomLfo behaves predictably.
 */
class RandomLfoTest : public ProcessorTest {
public:
    /**
     * @brief Constructs a new RandomLfoTest with the specified test name.
     */
    RandomLfoTest() : ProcessorTest("Random Lfo") { }

    /**
     * @brief Runs the RandomLfo test by performing input bounds checks, verifying that the
     *        LFO output remains finite and stable under extreme input conditions.
     */
    void runTest() override;
};
