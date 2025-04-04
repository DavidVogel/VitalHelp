/**
 * @file reverb_section_test.h
 * @brief Declares the ReverbSectionTest class, which tests the ReverbSection interface component.
 */

#pragma once

#include "interface_test.h"

/**
 * @class ReverbSectionTest
 * @brief A test class for verifying the functionality and reliability of the ReverbSection UI component.
 *
 * This test sets up a synth engine, instantiates a ReverbSection UI component, and runs stress tests
 * to ensure that the reverb parameters and UI interactions behave correctly, even under random or
 * extreme conditions.
 */
class ReverbSectionTest : public InterfaceTest {
public:
    /**
     * @brief Constructs a new ReverbSectionTest with a specified test name.
     */
    ReverbSectionTest() : InterfaceTest("Reverb Section") { }

    /**
     * @brief Runs the ReverbSection test by setting up the environment, creating the component,
     *        and executing stress tests against it.
     */
    void runTest() override;
};
