/**
 * @file comb_filter_test.h
 * @brief Declares the CombFilterTest class, which tests the CombFilter processor.
 */

#pragma once

#include "processor_test.h"

/**
 * @class CombFilterTest
 * @brief A test class that verifies the stability and correctness of the CombFilter processor.
 *
 * This test runs input bounds checks on the CombFilter under various filter styles. By doing so,
 * it ensures that the CombFilter remains stable and produces finite outputs across a wide range
 * of input conditions and filter types.
 */
class CombFilterTest : public ProcessorTest {
public:
    /**
     * @brief Constructs a new CombFilterTest with a specified test name.
     */
    CombFilterTest() : ProcessorTest("Comb Filter") { }

    /**
     * @brief Runs the comb filter test by cycling through all filter types,
     *        applying input bounds tests, and ensuring the output is stable.
     */
    void runTest() override;
};
