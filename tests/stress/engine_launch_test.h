/**
 * @file engine_launch_test.h
 * @brief Declares the EngineLaunchTest class, which is a stress test ensuring that the SoundEngine can be repeatedly launched and used reliably.
 */

#pragma once

#include "JuceHeader.h"

/**
 * @class EngineLaunchTest
 * @brief A stress test class that verifies the stability and correctness of launching and using multiple SoundEngine instances.
 *
 * This test creates several SoundEngine instances, triggers notes, processes audio, and checks for
 * finite output values. It ensures that launching multiple engines in succession doesn't lead to
 * instability or invalid numeric output.
 */
class EngineLaunchTest : public UnitTest {
public:
    /**
     * @brief Constructs an EngineLaunchTest with the specified test name and category.
     */
    EngineLaunchTest() : UnitTest("Engine Launch", "Stress") { }

    /**
     * @brief Runs the engine launch test, creating multiple SoundEngine instances and testing their behavior.
     */
    void runTest() override;

    /**
     * @brief Performs the actual launch test on multiple SoundEngine instances.
     */
    void launchTest();
};
