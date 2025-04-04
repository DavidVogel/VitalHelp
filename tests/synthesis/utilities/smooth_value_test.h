/**
 * @file smooth_value_test.h
 * @brief Declares the SmoothValueTest class, which tests the SmoothValue processor.
 */

#pragma once

#include "processor_test.h"

/**
 * @class SmoothValueTest
 * @brief A test class that verifies the stability and correctness of the SmoothValue processor.
 *
 * This test ensures that the SmoothValue processor handles extreme input conditions without producing
 * invalid (non-finite) outputs. By leveraging the ProcessorTest framework, it applies standardized input
 * bounds tests to verify that the smoothed output remains stable and finite.
 */
class SmoothValueTest : public ProcessorTest {
public:
    /**
     * @brief Constructs a new SmoothValueTest with the specified test name.
     */
    SmoothValueTest() : ProcessorTest("Smooth Value") { }

    /**
     * @brief Runs the SmoothValue test by performing input bounds checks, verifying that the
     *        smoothed value output remains stable and finite under extreme input conditions.
     */
    void runTest() override;
};
