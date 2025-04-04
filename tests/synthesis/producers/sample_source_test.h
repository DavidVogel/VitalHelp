/**
 * @file sample_source_test.h
 * @brief Declares the SampleSourceTest class, which tests the SampleSource processor.
 */

#pragma once

#include "processor_test.h"

/**
 * @class SampleSourceTest
 * @brief A test class that verifies the stability and correctness of the SampleSource processor.
 *
 * This test ensures that the SampleSource processor, which provides audio samples from a sample buffer,
 * handles extreme input conditions without producing invalid (non-finite) outputs. It uses the ProcessorTest
 * framework to run standardized input bounds tests.
 */
class SampleSourceTest : public ProcessorTest {
public:
    /**
     * @brief Constructs a new SampleSourceTest with the specified test name.
     */
    SampleSourceTest() : ProcessorTest("Sample Source") { }

    /**
     * @brief Runs the SampleSource test by performing input bounds checks, verifying that the
     *        processor output remains stable and finite under extreme input conditions.
     */
    void runTest() override;
};
