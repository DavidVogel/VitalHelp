/**
 * @file synthesis_interface_test.h
 * @brief Declares the SynthesisInterfaceTest class, which tests the SynthesisInterface UI component.
 */

#pragma once

#include "interface_test.h"

/**
 * @class SynthesisInterfaceTest
 * @brief A test class for verifying the functionality and reliability of the SynthesisInterface UI component.
 *
 * This test sets up a synth engine, creates a SynthesisInterface UI component, and then runs stress tests
 * to ensure that the synthesizer parameters and UI interactions behave correctly, even under random or
 * extreme conditions.
 */
class SynthesisInterfaceTest : public InterfaceTest {
public:
    /**
     * @brief Constructs a new SynthesisInterfaceTest with a specified test name.
     */
    SynthesisInterfaceTest() : InterfaceTest("Synthesis Interface") { }

    /**
     * @brief Runs the SynthesisInterface test by setting up the environment, creating the component,
     *        and executing stress tests against it.
     */
    void runTest() override;
};
