/**
 * @file equalizer_section_test.h
 * @brief Declares the EqualizerSectionTest class, which tests the EqualizerSection interface component.
 */

#pragma once

#include "interface_test.h"

/**
 * @class EqualizerSectionTest
 * @brief A test class for verifying the functionality and reliability of the EqualizerSection UI component.
 *
 * This test sets up a synth engine, creates an EqualizerSection, and runs stress tests
 * to ensure that the UI elements and their interactions with the underlying engine are stable and correct.
 */
class EqualizerSectionTest : public InterfaceTest {
public:
    /**
     * @brief Constructs a new EqualizerSectionTest with a specified test name.
     */
    EqualizerSectionTest() : InterfaceTest("Equalizer Section") { }

    /**
     * @brief Runs the EqualizerSection test by setting up the environment, creating the component,
     *        and subjecting it to stress tests.
     */
    void runTest() override;
};
