/**
 * @file flanger_section_test.h
 * @brief Declares the FlangerSectionTest class, which tests the FlangerSection interface component.
 */

#pragma once

#include "interface_test.h"

/**
 * @class FlangerSectionTest
 * @brief A test class for verifying the functionality and stability of the FlangerSection UI component.
 *
 * This class sets up a synth engine environment, creates a FlangerSection component, and runs
 * stress tests to ensure that the flanger parameters and UI interactions work correctly, even
 * under random or extreme conditions.
 */
class FlangerSectionTest : public InterfaceTest {
public:
    /**
     * @brief Constructs a new FlangerSectionTest with a specified test name.
     */
    FlangerSectionTest() : InterfaceTest("Flanger Section") { }

    /**
     * @brief Runs the FlangerSection test by setting up the environment, creating the component,
     *        and executing stress tests against it.
     */
    void runTest() override;
};
