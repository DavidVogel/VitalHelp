/**
 * @file oscillator_section_test.h
 * @brief Declares the OscillatorSectionTest class, which tests the OscillatorSection interface component.
 */

#pragma once

#include "interface_test.h"

/**
 * @class OscillatorSectionTest
 * @brief A test class for verifying the functionality and stability of the OscillatorSection UI component.
 *
 * This test sets up a synth engine, creates an OscillatorSection, and runs stress tests
 * to ensure that the oscillator parameters and UI interactions behave as expected, even under
 * random or extreme input conditions.
 */
class OscillatorSectionTest : public InterfaceTest {
public:
    /**
     * @brief Constructs a new OscillatorSectionTest with a specified test name.
     */
    OscillatorSectionTest() : InterfaceTest("Oscillator Section") { }

    /**
     * @brief Runs the OscillatorSection test by setting up the environment, creating the component,
     *        and executing stress tests against it.
     */
    void runTest() override;
};
