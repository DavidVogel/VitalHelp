/**
 * @file distortion_section_test.h
 * @brief Declares the DistortionSectionTest class, which tests the DistortionSection interface component.
 */

#pragma once

#include "interface_test.h"

/**
 * @class DistortionSectionTest
 * @brief A test class for verifying the functionality and reliability of the DistortionSection interface.
 *
 * This class sets up a synth engine, creates a DistortionSection UI component, and runs
 * stress tests to ensure that the distortion controls and UI elements function correctly under
 * various conditions.
 */
class DistortionSectionTest : public InterfaceTest {
public:
    /**
     * @brief Constructs a new DistortionSectionTest with a specified test name.
     */
    DistortionSectionTest() : InterfaceTest("Distortion Section") { }

    /**
     * @brief Runs the distortion section tests by setting up the environment,
     * creating the component, and subjecting it to stress tests.
     */
    void runTest() override;
};
