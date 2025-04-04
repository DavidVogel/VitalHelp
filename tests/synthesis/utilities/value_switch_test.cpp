/**
 * @file value_switch_test.cpp
 * @brief Implements the ValueSwitchTest class, performing input bounds tests on the ValueSwitch processor.
 */

#include "value_switch_test.h"
#include "value_switch.h"

void ValueSwitchTest::runTest() {
    // Create a ValueSwitch processor instance.
    vital::ValueSwitch value_switch;

    // Run a standardized input bounds test to ensure the value switch handles extreme inputs gracefully.
    runInputBoundsTest(&value_switch);
}

// Registers the test instance so it will be automatically discovered and run.
static ValueSwitchTest value_switch_test;
