/**
 * @file smooth_value_test.cpp
 * @brief Implements the SmoothValueTest class, performing input bounds tests on the SmoothValue processor.
 */

#include "smooth_value_test.h"
#include "smooth_value.h"

void SmoothValueTest::runTest() {
    // Create a SmoothValue processor instance.
    vital::SmoothValue smooth_value;

    // Run a standardized input bounds test to ensure the smooth value handles extreme inputs gracefully.
    runInputBoundsTest(&smooth_value);
}

// Registers the test instance so it will be automatically discovered and run.
static SmoothValueTest smooth_value_test;
