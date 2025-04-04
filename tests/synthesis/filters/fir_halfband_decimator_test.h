/**
 * @file fir_halfband_decimator_test.h
 * @brief Declares the FirHalfbandDecimatorTest class, which tests the FirHalfbandDecimator processor.
 */

#pragma once

#include "processor_test.h"

/**
 * @class FirHalfbandDecimatorTest
 * @brief A test class that, when fully implemented, will verify the stability and correctness of the FirHalfbandDecimator processor.
 *
 * Currently, this test is not fully implemented, as indicated by the TODO comment in the source file.
 * Once completed, it will ensure that the FirHalfbandDecimator handles extreme input conditions without
 * producing non-finite outputs, leveraging the ProcessorTest framework to run standardized input tests.
 */
class FirHalfbandDecimatorTest : public ProcessorTest {
public:
    /**
     * @brief Constructs a new FirHalfbandDecimatorTest with a specified test name.
     */
    FirHalfbandDecimatorTest() : ProcessorTest("Fir Halfband Decimator") { }

    /**
     * @brief Runs the FirHalfbandDecimator test.
     *
     * @note Currently, this test is not implemented. A call to runInputBoundsTest()
     *       is commented out and should be enabled or expanded upon once the processor
     *       is ready for testing.
     */
    void runTest() override;
};
