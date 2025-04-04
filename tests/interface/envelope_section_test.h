/**
 * @file envelope_section_test.h
 * @brief Declares the EnvelopeSectionTest class, which tests the EnvelopeSection interface component.
 */

#pragma once

#include "interface_test.h"

/**
 * @class EnvelopeSectionTest
 * @brief A test class for verifying the functionality and reliability of the EnvelopeSection UI component.
 *
 * This class instantiates a synth engine, creates an EnvelopeSection UI component, and runs stress tests
 * to ensure that envelope parameters and UI interactions work correctly, even under extreme or random conditions.
 */
class EnvelopeSectionTest : public InterfaceTest {
public:
    /**
     * @brief Constructs a new EnvelopeSectionTest with a specified test name.
     */
    EnvelopeSectionTest() : InterfaceTest("Envelope Section") { }

    /**
     * @brief Runs the EnvelopeSection test by setting up the environment, creating the component,
     *        and executing stress tests against it.
     */
    void runTest() override;
};
