/**
 * @file oscillator_advanced_section_test.h
 * @brief Declares the OscillatorAdvancedSectionTest class, which tests the OscillatorAdvancedSection interface component.
 */

#pragma once

#include "interface_test.h"

/**
 * @class OscillatorAdvancedSectionTest
 * @brief A test class for verifying the functionality and reliability of the OscillatorAdvancedSection UI component.
 *
 * This test sets up a synth engine, instantiates an OscillatorAdvancedSection, and runs stress tests
 * to ensure that the advanced oscillator parameters and interactions behave correctly, even under
 * random or extreme conditions.
 */
class OscillatorAdvancedSectionTest : public InterfaceTest {
public:
    /**
     * @brief Constructs a new OscillatorAdvancedSectionTest with a specified test name.
     */
    OscillatorAdvancedSectionTest() : InterfaceTest("Oscillator Advanced Section") { }

    /**
     * @brief Runs the OscillatorAdvancedSection test by setting up the environment, creating the component,
     *        and executing stress tests against it.
     */
    void runTest() override;
};
