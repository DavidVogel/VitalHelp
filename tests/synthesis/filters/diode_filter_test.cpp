/**
 * @file diode_filter_test.cpp
 * @brief Implements the DiodeFilterTest class, performing input bounds tests on the DiodeFilter processor.
 */

#include "diode_filter_test.h"
#include "diode_filter.h"

void DiodeFilterTest::runTest() {
    // Create a DiodeFilter processor instance.
    vital::DiodeFilter diode_filter;

    // Run standardized input bounds tests to ensure the diode filter handles extreme inputs gracefully.
    runInputBoundsTest(&diode_filter);
}

// Registers the test instance so it will be automatically discovered and run.
static DiodeFilterTest diode_filter_test;
