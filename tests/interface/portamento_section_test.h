/**
 * @file portamento_section_test.h
 * @brief Declares the PortamentoSectionTest class, which tests the PortamentoSection interface component.
 */

#pragma once

#include "interface_test.h"

/**
 * @class PortamentoSectionTest
 * @brief A test class for verifying the functionality and reliability of the PortamentoSection UI component.
 *
 * This test sets up a synth engine and instantiates a PortamentoSection UI component, then runs stress tests
 * to ensure that portamento parameters and UI interactions behave correctly, even under random or extreme conditions.
 */
class PortamentoSectionTest : public InterfaceTest {
public:
    /**
     * @brief Constructs a new PortamentoSectionTest with a specified test name.
     */
    PortamentoSectionTest() : InterfaceTest("Portamento Section") { }

    /**
     * @brief Runs the PortamentoSection test by setting up the environment, creating the component,
     *        and executing stress tests against it.
     */
    void runTest() override;
};
