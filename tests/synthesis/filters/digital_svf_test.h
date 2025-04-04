/**
 * @file digital_svf_test.h
 * @brief Declares the DigitalSvfTest class, which tests the DigitalSvf processor.
 */

#pragma once

#include "processor_test.h"

/**
 * @class DigitalSvfTest
 * @brief A test class that verifies the stability and correctness of the Digital State Variable Filter (SVF).
 *
 * This test ensures that the Digital SVF processor handles extreme input values without producing
 * non-finite outputs. It leverages the ProcessorTest framework to conduct standardized input bounds tests.
 */
class DigitalSvfTest : public ProcessorTest {
public:
    /**
     * @brief Constructs a new DigitalSvfTest with the specified test name.
     */
    DigitalSvfTest() : ProcessorTest("Digital SVF") { }

    /**
     * @brief Runs the Digital SVF test by performing input bounds checks, verifying that the
     *        filter remains stable and produces finite output under extreme conditions.
     */
    void runTest() override;
};
