/**
 * @file iir_halfband_decimator_test.h
 * @brief Declares the IirHalfbandDecimatorTest class, which tests the IirHalfbandDecimator processor.
 */

#pragma once

#include "processor_test.h"

/**
 * @class IirHalfbandDecimatorTest
 * @brief A test class that, once fully implemented, will verify the stability and correctness of the IirHalfbandDecimator processor.
 *
 * As indicated by the TODO comment in the source file, this test is currently not fully implemented.
 * Once completed, it will ensure that the IirHalfbandDecimator handles extreme input conditions without
 * producing non-finite outputs, leveraging the ProcessorTest framework for standardized input testing.
 */
class IirHalfbandDecimatorTest : public ProcessorTest {
public:
    /**
     * @brief Constructs a new IirHalfbandDecimatorTest with a specified test name.
     */
    IirHalfbandDecimatorTest() : ProcessorTest("Iir Halfband Decimator") { }

    /**
     * @brief Runs the IirHalfbandDecimator test.
     *
     * @note Currently, this test is not implemented. The call to runInputBoundsTest()
     *       is commented out. Once the processor is ready for full testing, enable and/or
     *       expand the test accordingly.
     */
    void runTest() override;
};
