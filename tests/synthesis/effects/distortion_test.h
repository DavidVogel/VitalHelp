/**
 * @file distortion_test.h
 * @brief Declares the DistortionTest class, which tests the Distortion processor.
 */

#pragma once

#include "processor_test.h"

/**
 * @class DistortionTest
 * @brief A test class for verifying the behavior and correctness of the Distortion processor.
 *
 * This test ensures that the Distortion processor handles a wide range of input values without producing
 * invalid outputs (such as NaNs or infinities). It uses the ProcessorTest framework to run standardized
 * input bounds tests, ignoring certain outputs that aren't relevant to the main distortion output.
 */
class DistortionTest : public ProcessorTest {
public:
    /**
     * @brief Constructs a new DistortionTest with a specified test name.
     */
    DistortionTest() : ProcessorTest("Distortion") { }

    /**
     * @brief Runs the Distortion test by performing input bounds checks, verifying that the distortion
     *        output remains valid under extreme input conditions.
     */
    void runTest() override;
};
