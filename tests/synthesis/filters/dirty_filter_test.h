/**
 * @file dirty_filter_test.h
 * @brief Declares the DirtyFilterTest class, which tests the DirtyFilter processor.
 */

#pragma once

#include "processor_test.h"

/**
 * @class DirtyFilterTest
 * @brief A test class that verifies the stability and correctness of the DirtyFilter processor.
 *
 * This test checks whether the DirtyFilter processor can handle extreme input conditions without producing
 * invalid (non-finite) outputs. It uses the ProcessorTest framework to run standardized input bounds tests.
 */
class DirtyFilterTest : public ProcessorTest {
public:
    /**
     * @brief Constructs a new DirtyFilterTest with the specified test name.
     */
    DirtyFilterTest() : ProcessorTest("Dirty Filter") { }

    /**
     * @brief Runs the DirtyFilter test by performing input bounds checks, ensuring that the filter
     *        remains stable and produces finite output under extreme input conditions.
     */
    void runTest() override;
};
