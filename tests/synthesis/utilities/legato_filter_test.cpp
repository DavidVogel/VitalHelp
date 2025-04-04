/**
 * @file legato_filter_test.cpp
 * @brief Implements the LegatoFilterTest class, performing input bounds tests on the LegatoFilter processor.
 */

#include "legato_filter_test.h"
#include "legato_filter.h"

void LegatoFilterTest::runTest() {
    // Create a LegatoFilter processor instance.
    vital::LegatoFilter legato_filter;

    // Run a standardized input bounds test to ensure the legato filter handles extreme inputs gracefully.
    runInputBoundsTest(&legato_filter);
}

// Registers the test instance so it will be automatically discovered and run.
static LegatoFilterTest legato_filter_test;
