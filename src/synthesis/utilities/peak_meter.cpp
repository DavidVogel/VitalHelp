/**
 * @file peak_meter.cpp
 * @brief Implementation of the PeakMeter class, handling real-time peak and memory-peak calculations.
 */

#include "peak_meter.h"
#include "utils.h"
#include "synth_constants.h"

namespace vital {

    namespace {
        /// Decay rate for the instantaneous sample peak measurement.
        constexpr mono_float kSampleDecay = 8096.0f;
        /// Decay rate for the remembered peak measurement.
        constexpr mono_float kRememberedDecay = 20000.0f;
        /// Hold duration (in samples) for the remembered peak before it starts decaying.
        constexpr mono_float kRememberedHold = 50000.0f;
    } // namespace

    PeakMeter::PeakMeter() :
            Processor(1, 2),
            current_peak_(0.0f),
            current_square_sum_(0.0f),
            remembered_peak_(0.0f),
            samples_since_remembered_(0) { }

    void PeakMeter::process(int num_samples) {
        // Get input audio buffer.
        const poly_float* audio_in = input(0)->source->buffer;

        // Compute peak value of the current block.
        poly_float peak = utils::peak(audio_in, num_samples);

        // Compute sample-based decay rates depending on oversampling.
        mono_float samples = getOversampleAmount() * kSampleDecay;
        mono_float mult = (samples - 1.0f) / samples;
        poly_float current_peak = current_peak_;

        mono_float remembered_samples = getOversampleAmount() * kRememberedDecay;
        mono_float remembered_mult = (remembered_samples - 1.0f) / remembered_samples;
        poly_float current_remembered_peak = remembered_peak_;

        poly_float current_square_sum = current_square_sum_;

        // Apply decay to peak and remembered peak, accumulate squared samples.
        for (int i = 0; i < num_samples; ++i) {
            current_peak *= mult;
            current_remembered_peak *= remembered_mult;
            current_square_sum *= mult;
            poly_float sample = audio_in[i];
            current_square_sum += sample * sample;
        }

        current_peak_ = utils::max(current_peak, peak);
        samples_since_remembered_ += num_samples;

        // Update remembered peak logic:
        // If the current peak is less than the remembered peak, we keep counting samples since remembered.
        samples_since_remembered_ &= poly_float::lessThan(current_peak_, current_remembered_peak);

        mono_float remembered_hold_samples = getOversampleAmount() * kRememberedHold;
        poly_mask hold_mask = poly_int::lessThan(samples_since_remembered_, remembered_hold_samples);
        current_remembered_peak = utils::maskLoad(current_remembered_peak, remembered_peak_, hold_mask);

        remembered_peak_ = utils::max(current_peak_, current_remembered_peak);
        current_square_sum_ = current_square_sum;

        // Compute RMS as needed (though here we just store info).
        poly_float rms = utils::sqrt(current_square_sum_ * (1.0f / samples));
        poly_float prepped_rms = utils::swapVoices(rms);

        // Write outputs: current peak and remembered peak.
        output(kLevel)->buffer[0] = utils::maskLoad(prepped_rms, current_peak_, constants::kFirstMask);
        output(kMemoryPeak)->buffer[0] = remembered_peak_;
    }
} // namespace vital
