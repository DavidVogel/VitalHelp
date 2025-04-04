/**
 * @file formant_filter_test.cpp
 * @brief Implements the FormantFilterTest class, performing input bounds tests on the FormantFilter processor.
 */

#include "formant_filter_test.h"
#include "formant_filter.h"

void FormantFilterTest::runTest() {
    // Create a FormantFilter processor instance.
    vital::FormantFilter formant_filter;

    // Run standardized input bounds tests to ensure the formant filter handles extreme inputs gracefully.
    runInputBoundsTest(&formant_filter);
}

// Registers the test instance so it will be automatically discovered and run.
static FormantFilterTest formant_filter_test;
