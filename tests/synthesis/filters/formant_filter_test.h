/**
 * @file formant_filter_test.h
 * @brief Declares the FormantFilterTest class, which tests the FormantFilter processor.
 */

#pragma once

#include "processor_test.h"

/**
 * @class FormantFilterTest
 * @brief A test class that verifies the stability and correctness of the FormantFilter processor.
 *
 * This test ensures that the FormantFilter handles a wide range of input values without producing
 * non-finite outputs. It leverages the ProcessorTest framework to run standardized input bounds tests.
 */
class FormantFilterTest : public ProcessorTest {
public:
    /**
     * @brief Constructs a new FormantFilterTest with the specified test name.
     */
    FormantFilterTest() : ProcessorTest("Formant Filter") { }

    /**
     * @brief Runs the FormantFilter test by performing input bounds checks, verifying that the
     *        filter remains stable and produces finite output under extreme input conditions.
     */
    void runTest() override;
};
