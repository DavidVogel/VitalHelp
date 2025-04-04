/**
 * @file portamento_slope.cpp
 * @brief Implements the PortamentoSlope class, handling the logic for transitioning between source and target values.
 */

#include "portamento_slope.h"
#include "utils.h"
#include "futils.h"

namespace vital {

    PortamentoSlope::PortamentoSlope() :
            Processor(kNumInputs, 1, true),
            position_(0.0f) { }

    void PortamentoSlope::processBypass(int start) {
        // If bypassed (no portamento), set position to 1.0 (fully at target)
        position_ = 1.0f;
        // Directly output target value as no portamento is occurring.
        output()->buffer[0] = input(kTarget)->source->buffer[0];
    }

    void PortamentoSlope::process(int num_samples) {
        bool force = input(kPortamentoForce)->at(0)[0];
        poly_float run_seconds = input(kRunSeconds)->at(0);

        // Determine if the portamento is active (run_seconds > minimum)
        poly_mask active_mask = poly_float::greaterThan(run_seconds, kMinPortamentoTime);
        if (active_mask.anyMask() == 0) {
            // If not active, just bypass.
            processBypass(0);
            return;
        }

        // Check for reset triggers.
        poly_mask reset_mask = getResetMask(kReset);
        position_ = utils::maskLoad(position_, 0.0f, reset_mask);

        // If not forcing portamento, consider the number of notes pressed.
        if (!force) {
            poly_float num_voices = input(kNumNotesPressed)->at(0);
            reset_mask = reset_mask & poly_float::equal(num_voices, 1.0f);
            position_ = utils::maskLoad(position_, 1.0f, reset_mask);
        }

        poly_float target = input(kTarget)->at(0);
        poly_float source = input(kSource)->at(0);

        // If scaling is enabled, adjust the run_seconds based on pitch interval.
        if (input(kPortamentoScale)->at(0)[0]) {
            poly_float midi_delta = poly_float::abs(target - source);
            // Adjust run time by interval in octaves (or notes), defined in kNotesPerOctave.
            run_seconds *= midi_delta * (1.0f / kNotesPerOctave);
        }

        // Compute how much to move per sample.
        poly_float position_delta = poly_float(1.0f * num_samples) / (run_seconds * getSampleRate());
        position_ = utils::clamp(position_ + position_delta, 0.0f, 1.0f);

        // Apply power scaling to the position if a slope power is provided.
        poly_float power = -input(kSlopePower)->at(0);
        poly_float adjusted_position = futils::powerScale(position_, power);

        // Interpolate between source and target based on adjusted position.
        output()->buffer[0] = utils::interpolate(source, target, adjusted_position);
    }
} // namespace vital
