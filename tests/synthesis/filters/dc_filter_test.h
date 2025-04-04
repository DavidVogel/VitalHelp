/**
 * @file dc_filter_test.h
 * @brief Declares the DcFilterTest class, which tests the DcFilter processor.
 */

#pragma once

#include "processor_test.h"

/**
 * @class DcFilterTest
 * @brief A test class that verifies the stability and correctness of the DcFilter processor.
 *
 * This test ensures that the DC Filter processor handles a wide range of input values without producing
 * non-finite outputs. It leverages the ProcessorTest framework to perform input bounds tests.
 */
class DcFilterTest : public ProcessorTest {
public:
    /**
     * @brief Constructs a new DcFilterTest with a specified test name.
     */
    DcFilterTest() : ProcessorTest("DC Filter") { }

    /**
     * @brief Runs the DC Filter test by performing input bounds checks and verifying the processor's stability.
     */
    void runTest() override;
};
