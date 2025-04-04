/**
 * @file filter_section_test.h
 * @brief Declares the FilterSectionTest class, which tests the FilterSection interface component.
 */

#pragma once

#include "interface_test.h"

/**
 * @class FilterSectionTest
 * @brief A test class for verifying the functionality and stability of the FilterSection UI component.
 *
 * This class sets up a synth engine, instantiates a FilterSection UI component,
 * and runs stress tests to ensure the filter parameters and interactions behave correctly,
 * even under random or extreme conditions.
 */
class FilterSectionTest : public InterfaceTest {
public:
    /**
     * @brief Constructs a new FilterSectionTest with a specified test name.
     */
    FilterSectionTest() : InterfaceTest("Filter Section") { }

    /**
     * @brief Runs the FilterSection test by setting up the environment, creating the filter section,
     *        and executing stress tests against it.
     */
    void runTest() override;
};
