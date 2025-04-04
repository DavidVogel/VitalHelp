/**
 * @file lfo_section_test.h
 * @brief Declares the LfoSectionTest class, which tests the LfoSection interface component.
 */

#pragma once

#include "interface_test.h"

/**
 * @class LfoSectionTest
 * @brief A test class for verifying the functionality and reliability of the LfoSection UI component.
 *
 * This test sets up a synth engine, creates an LFO (LineGenerator) source, and then
 * instantiates an LfoSection UI component. It runs stress tests to ensure that the LFO parameters
 * and interactions work correctly, even under random or extreme conditions.
 */
class LfoSectionTest : public InterfaceTest {
public:
    /**
     * @brief Constructs a new LfoSectionTest with a specified test name.
     */
    LfoSectionTest() : InterfaceTest("Lfo Section") { }

    /**
     * @brief Runs the LfoSection test by setting up the environment, creating the component,
     *        and executing stress tests against it.
     */
    void runTest() override;
};
