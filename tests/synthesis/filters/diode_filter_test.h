/**
 * @file diode_filter_test.h
 * @brief Declares the DiodeFilterTest class, which tests the DiodeFilter processor.
 */

#pragma once

#include "processor_test.h"

/**
 * @class DiodeFilterTest
 * @brief A test class that verifies the stability and correctness of the DiodeFilter processor.
 *
 * This test ensures that the DiodeFilter processor handles extreme input values without producing
 * invalid outputs. It leverages the ProcessorTest framework to run standardized input bounds tests.
 */
class DiodeFilterTest : public ProcessorTest {
public:
    /**
     * @brief Constructs a new DiodeFilterTest with the specified test name.
     */
    DiodeFilterTest() : ProcessorTest("Diode Filter") { }

    /**
     * @brief Runs the DiodeFilter test by performing input bounds checks, verifying that the
     *        filter remains stable and produces finite output under extreme input conditions.
     */
    void runTest() override;
};
