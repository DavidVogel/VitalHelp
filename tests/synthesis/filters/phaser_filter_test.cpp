/**
 * @file phaser_filter_test.cpp
 * @brief Implements the PhaserFilterTest class, performing input bounds tests on the PhaserFilter processor.
 */

#include "phaser_filter_test.h"
#include "phaser_filter.h"

void PhaserFilterTest::runTest() {
    // Create a PhaserFilter processor with inversion set to true and test it.
    vital::PhaserFilter phaser_filter1(true);
    runInputBoundsTest(&phaser_filter1);

    // Create another PhaserFilter processor with inversion set to false and test it.
    vital::PhaserFilter phaser_filter2(false);
    runInputBoundsTest(&phaser_filter2);
}

// Registers the test instance so it will be automatically discovered and run.
static PhaserFilterTest phaser_filter_test;
