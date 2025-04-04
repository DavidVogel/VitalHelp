/**
 * @file delay_test.h
 * @brief Declares the DelayTest class, which tests delay-related processors.
 */

#pragma once

#include "processor_test.h"

/**
 * @class DelayTest
 * @brief A test class for verifying the behavior and correctness of delay processors.
 *
 * This class tests various delay processors, such as MultiDelay and StereoDelay, to ensure they
 * produce stable output and handle extreme input values properly. By extending ProcessorTest,
 * it leverages standardized testing methods like input bounds tests.
 */
class DelayTest : public ProcessorTest {
public:
    /**
     * @brief Constructs a new DelayTest with the specified test name.
     */
    DelayTest() : ProcessorTest("Delay") { }

    /**
     * @brief Runs the delay test by performing input bounds checks on the tested delay processors.
     */
    void runTest() override;
};
