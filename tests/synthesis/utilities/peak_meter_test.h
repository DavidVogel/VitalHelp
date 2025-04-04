/**
 * @file peak_meter_test.h
 * @brief Declares the PeakMeterTest class, which tests the PeakMeter processor.
 */

#pragma once

#include "processor_test.h"

/**
 * @class PeakMeterTest
 * @brief A test class that verifies the stability and correctness of the PeakMeter processor.
 *
 * This test ensures that the PeakMeter processor handles extreme input conditions without producing
 * invalid (non-finite) outputs. By leveraging the ProcessorTest framework, it applies standardized
 * input bounds tests to ensure the meter behaves predictably, accurately tracking peak levels.
 */
class PeakMeterTest : public ProcessorTest {
public:
    /**
     * @brief Constructs a new PeakMeterTest with the specified test name.
     */
    PeakMeterTest() : ProcessorTest("Peak Meter") { }

    /**
     * @brief Runs the PeakMeter test by performing input bounds checks, verifying that the
     *        peak meter output remains stable and finite under extreme input conditions.
     */
    void runTest() override;
};
