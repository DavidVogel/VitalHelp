/**
 * @file delay_test.cpp
 * @brief Implements the DelayTest class, performing input bounds tests on delay processors.
 */

#include "delay_test.h"
#include "delay.h"
#include "memory.h"

void DelayTest::runTest() {
    // Create delay processors with a specific memory size.
    vital::MultiDelay multi_delay(10000);
    vital::StereoDelay stereo_delay(10000);

    // Run input bounds tests on the multi_delay and stereo_delay processors
    // to ensure they handle extreme input conditions gracefully.
    runInputBoundsTest(&multi_delay);
    runInputBoundsTest(&stereo_delay);
}

// Registers the test instance so it will be automatically discovered and run.
static DelayTest delay_test;
