/**
 * @file sallen_key_filter_test.cpp
 * @brief Implements the SallenKeyFilterTest class, performing input bounds tests on the SallenKeyFilter processor.
 */

#include "sallen_key_filter_test.h"
#include "sallen_key_filter.h"

#include <cmath>

void SallenKeyFilterTest::runTest() {
    // Create a SallenKeyFilter processor instance.
    vital::SallenKeyFilter sallen_key_filter;

    // Run standardized input bounds tests to ensure the Sallen-Key filter handles extreme inputs gracefully.
    runInputBoundsTest(&sallen_key_filter);
}

// Registers the test instance so it will be automatically discovered and run.
static SallenKeyFilterTest sallen_key_filter_test;
