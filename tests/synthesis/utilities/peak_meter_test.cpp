/**
 * @file peak_meter_test.cpp
 * @brief Implements the PeakMeterTest class, performing input bounds tests on the PeakMeter processor.
 */

#include "peak_meter_test.h"
#include "peak_meter.h"

void PeakMeterTest::runTest() {
    // Create a PeakMeter processor instance.
    vital::PeakMeter peak_meter;

    // Run standardized input bounds tests to ensure the peak meter handles extreme inputs gracefully.
    runInputBoundsTest(&peak_meter);
}

// Registers the test instance so it will be automatically discovered and run.
static PeakMeterTest peak_meter_test;
