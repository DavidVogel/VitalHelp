/**
 * @file linkwitz_riley_filter_test.cpp
 * @brief Implements the LinkwitzRileyFilterTest class, performing input bounds tests on the LinkwitzRileyFilter processor.
 */

#include "linkwitz_riley_filter_test.h"
#include "linkwitz_riley_filter.h"

void LinkwitzRileyFilterTest::runTest() {
    // Create a LinkwitzRileyFilter processor instance with a specified cutoff frequency.
    vital::LinkwitzRileyFilter linkwitz_riley_filter(500.0f);

    // Run standardized input bounds tests to ensure the Linkwitz-Riley filter handles extreme inputs gracefully.
    runInputBoundsTest(&linkwitz_riley_filter);
}

// Registers the test instance so it will be automatically discovered and run.
static LinkwitzRileyFilterTest linkwitz_riley_filter_test;
