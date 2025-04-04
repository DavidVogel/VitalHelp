/**
 * @file compressor_test.h
 * @brief Declares the CompressorTest class, which tests the Compressor processor.
 */

#pragma once

#include "processor_test.h"

/**
 * @class CompressorTest
 * @brief A test class for verifying the behavior and correctness of the Compressor processor.
 *
 * This test ensures that the Compressor processor handles various input conditions without producing
 * invalid or unexpected output. It uses the ProcessorTest base class to run standardized tests
 * such as input bounds testing.
 */
class CompressorTest : public ProcessorTest {
public:
    /**
     * @brief Constructs a new CompressorTest instance with a given test name.
     */
    CompressorTest() : ProcessorTest("Compressor") { }

    /**
     * @brief Runs the compressor test by performing input bounds checks and verifying the compressor's stability.
     */
    void runTest() override;
};
