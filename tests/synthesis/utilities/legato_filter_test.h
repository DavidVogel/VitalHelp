/**
 * @file legato_filter_test.h
 * @brief Declares the LegatoFilterTest class, which tests the LegatoFilter processor.
 */

#pragma once

#include "processor_test.h"

/**
 * @class LegatoFilterTest
 * @brief A test class that verifies the stability and correctness of the LegatoFilter processor.
 *
 * This test checks whether the LegatoFilter processor handles extreme input conditions without producing
 * invalid (non-finite) outputs. By leveraging the ProcessorTest framework, it applies standardized input
 * bounds tests to ensure the legato filtering behaves predictably.
 */
class LegatoFilterTest : public ProcessorTest {
public:
    /**
     * @brief Constructs a new LegatoFilterTest with the specified test name.
     */
    LegatoFilterTest() : ProcessorTest("Legato Filter") { }

    /**
     * @brief Runs the LegatoFilter test by performing input bounds checks, verifying that the filter's output
     *        remains stable and finite under extreme input conditions.
     */
    void runTest() override;
};
