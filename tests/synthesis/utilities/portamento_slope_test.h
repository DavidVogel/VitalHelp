/**
 * @file portamento_slope_test.h
 * @brief Declares the PortamentoSlopeTest class, which tests the PortamentoSlope processor.
 */

#pragma once

#include "processor_test.h"

/**
 * @class PortamentoSlopeTest
 * @brief A test class that, once fully implemented, will verify the stability and correctness of the PortamentoSlope processor.
 *
 * Currently, the input bounds test call is commented out. Once the PortamentoSlope is fully ready and
 * the expected behavior is defined, the input bounds test and other relevant checks can be enabled.
 */
class PortamentoSlopeTest : public ProcessorTest {
public:
    /**
     * @brief Constructs a new PortamentoSlopeTest with the specified test name.
     */
    PortamentoSlopeTest() : ProcessorTest("Portamento Slope") { }

    /**
     * @brief Runs the PortamentoSlope test.
     *
     * @note The actual input bounds test is currently commented out. Once the slope processor is ready for testing,
     *       uncomment and/or add the relevant testing code.
     */
    void runTest() override;
};
