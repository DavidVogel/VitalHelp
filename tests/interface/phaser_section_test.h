/**
 * @file phaser_section_test.h
 * @brief Declares the PhaserSectionTest class, which tests the PhaserSection interface component.
 */

#pragma once

#include "interface_test.h"

/**
 * @class PhaserSectionTest
 * @brief A test class for verifying the functionality and reliability of the PhaserSection UI component.
 *
 * This test sets up a synth engine, instantiates a PhaserSection UI component, and runs stress tests
 * to ensure that the phaser parameters and UI interactions behave correctly, even under random or
 * extreme conditions.
 */
class PhaserSectionTest : public InterfaceTest {
public:
    /**
     * @brief Constructs a new PhaserSectionTest with a specified test name.
     */
    PhaserSectionTest() : InterfaceTest("Phaser Section") { }

    /**
     * @brief Runs the PhaserSection test by setting up the environment, creating the component,
     *        and executing stress tests against it.
     */
    void runTest() override;
};
