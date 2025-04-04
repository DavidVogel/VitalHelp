/**
 * @file ladder_filter_test.h
 * @brief Declares the LadderFilterTest class, which tests the LadderFilter processor.
 */

#pragma once

#include "processor_test.h"

/**
 * @class LadderFilterTest
 * @brief A test class that verifies the stability and correctness of the LadderFilter processor.
 *
 * This test ensures that the LadderFilter handles extreme input values without producing
 * non-finite outputs. It leverages the ProcessorTest framework to run standardized input bounds tests.
 */
class LadderFilterTest : public ProcessorTest {
public:
    /**
     * @brief Constructs a new LadderFilterTest with the specified test name.
     */
    LadderFilterTest() : ProcessorTest("Ladder Filter") { }

    /**
     * @brief Runs the LadderFilter test by performing input bounds checks, verifying that the
     *        filter remains stable and produces finite output under extreme input conditions.
     */
    void runTest() override;
};
