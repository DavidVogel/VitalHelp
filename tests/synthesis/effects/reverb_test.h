/**
 * @file reverb_test.h
 * @brief Declares the ReverbTest class, which tests the Reverb processor.
 */

#pragma once

#include "processor_test.h"

/**
 * @class ReverbTest
 * @brief A test class that verifies the stability and correctness of the Reverb processor.
 *
 * This test ensures that the Reverb processor produces valid (finite) outputs across a wide range of input values.
 * By leveraging the ProcessorTest framework, it applies standardized tests such as input bounds checks.
 */
class ReverbTest : public ProcessorTest {
public:
    /**
     * @brief Constructs a new ReverbTest with a specified test name.
     */
    ReverbTest() : ProcessorTest("Reverb") { }

    /**
     * @brief Runs the Reverb test by performing input bounds checks and verifying the processor's stability
     *        under extreme conditions.
     */
    void runTest() override;
};
