/**
 * @file digital_svf_test.cpp
 * @brief Implements the DigitalSvfTest class, performing input bounds tests on the Digital SVF processor.
 */

#include "digital_svf_test.h"
#include "digital_svf.h"

void DigitalSvfTest::runTest() {
    // Create a DigitalSvf processor instance.
    vital::DigitalSvf digital_svf;

    // Run standardized input bounds tests on the digital SVF to ensure it handles extreme inputs gracefully.
    runInputBoundsTest(&digital_svf);
}

// Registers the test instance so it will be automatically discovered and run.
static DigitalSvfTest digital_svf_test;
