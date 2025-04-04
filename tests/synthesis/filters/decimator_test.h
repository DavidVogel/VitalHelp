/**
 * @file decimator_test.h
 * @brief Declares the DecimatorTest class, which tests the Decimator processor.
 */

#pragma once

#include "processor_test.h"

/**
 * @class DecimatorTest
 * @brief A test class for verifying the stability and correctness of the Decimator processor.
 *
 * This test ensures that the Decimator processor, which performs sample rate reduction and bit crushing,
 * handles a wide range of input values without producing non-finite outputs. It uses the ProcessorTest
 * framework to run standardized input bounds tests.
 */
class DecimatorTest : public ProcessorTest {
public:
    /**
     * @brief Constructs a new DecimatorTest with a specified test name.
     */
    DecimatorTest() : ProcessorTest("Decimator") { }

    /**
     * @brief Runs the Decimator test by performing input bounds checks, ensuring that the output remains finite
     *        and stable under extreme input conditions.
     */
    void runTest() override;
};
