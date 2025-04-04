/**
 * @file sallen_key_filter_test.h
 * @brief Declares the SallenKeyFilterTest class, which tests the SallenKeyFilter processor.
 */

#pragma once

#include "processor_test.h"

/**
 * @class SallenKeyFilterTest
 * @brief A test class that verifies the stability and correctness of the SallenKeyFilter processor.
 *
 * This test ensures that the SallenKeyFilter handles extreme input values without producing
 * non-finite outputs. It leverages the ProcessorTest framework to run standardized input bounds tests.
 */
class SallenKeyFilterTest : public ProcessorTest {
public:
    /**
     * @brief Constructs a new SallenKeyFilterTest with the specified test name.
     */
    SallenKeyFilterTest() : ProcessorTest("Sallen Key Filter") { }

    /**
     * @brief Runs the SallenKeyFilter test by performing input bounds checks, verifying that the
     *        filter remains stable and produces finite output under extreme input conditions.
     */
    void runTest() override;
};
