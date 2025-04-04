/**
 * @file sample_section_test.h
 * @brief Declares the SampleSectionTest class, which tests the SampleSection interface component.
 */

#pragma once

#include "interface_test.h"

/**
 * @class SampleSectionTest
 * @brief A test class for verifying the functionality and reliability of the SampleSection UI component.
 *
 * This test sets up a synth engine, creates a SampleSection UI component, and runs stress tests to ensure that
 * the sample loading and editing parameters, as well as UI interactions, behave correctly,
 * even under random or extreme conditions.
 */
class SampleSectionTest : public InterfaceTest {
public:
    /**
     * @brief Constructs a new SampleSectionTest with a specified test name.
     */
    SampleSectionTest() : InterfaceTest("Sample Section") { }

    /**
     * @brief Runs the SampleSection test by setting up the environment, creating the component,
     *        and executing stress tests against it.
     */
    void runTest() override;
};
