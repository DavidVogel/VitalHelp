/**
 * @file chorus_section_test.h
 * @brief Declares the ChorusSectionTest class used to test the ChorusSection interface component.
 */

#pragma once

#include "interface_test.h"

/**
 * @class ChorusSectionTest
 * @brief A test class that verifies the functionality and reliability of the ChorusSection UI component.
 *
 * This class sets up a sound engine and a ChorusSection component, then performs stress tests to
 * ensure that the UI interacts correctly with the engine and handles random inputs robustly.
 */
class ChorusSectionTest : public InterfaceTest {
public:
    /**
     * @brief Constructs a new ChorusSectionTest with a specified test name.
     */
    ChorusSectionTest() : InterfaceTest("Chorus Section") { }

    /**
     * @brief Runs the ChorusSection test by creating the test environment, setting up the component,
     *        and running stress tests against it.
     */
    void runTest() override;
};
