/*
Summary:
The PitchDetector class attempts to find the fundamental period of a loaded signal using a method inspired by the YIN algorithm. By computing an error metric that measures how well the waveform repeats at different periods, it identifies a period length that best represents the fundamental frequency. This helps ensure that wavetables and other audio processing steps align wave cycles with their fundamental pitch.
 */

#include "pitch_detector.h"
#include "synth_constants.h"
#include "wave_frame.h"

#include <climits>

PitchDetector::PitchDetector() {
  size_ = 0;
  signal_data_ = nullptr;
}

void PitchDetector::loadSignal(const float* signal, int size) {
  size_ = size;
  signal_data_ = std::make_unique<float[]>(size);

  memcpy(signal_data_.get(), signal, sizeof(float) * size);
}

float PitchDetector::getPeriodError(float period) {
  // This function measures how repeating a given period is within the signal.
  // It segments the signal into 'waves' (integral multiples of the period) and compares them.
  // It also penalizes DC differences by considering the mean offsets.
  static constexpr float kDcDeltaErrorMultiplier = 0.015f;
  float error = 0.0f;
  int waves = size_ / period - 1;
  VITAL_ASSERT(waves > 0);
  int points = kNumPoints / waves;
  for (int w = 0; w < waves; ++w) {
    float total_from = 0.0f;
    float total_to = 0.0f;
    for (int i = 0; i < points; ++i) {
      float first_position = w * period + i * period / points;
      float second_position = (w + 1) * period + i * period / points;

      int first_index = first_position;
      float first_t = first_position - first_index;
      float first_from = signal_data_[first_index];
      float first_to = signal_data_[first_index + 1];
      float first_value = vital::utils::interpolate(first_from, first_to, first_t);
      total_from += first_value;

      int second_index = second_position;
      float second_t = second_position - second_index;
      float second_from = signal_data_[second_index];
      float second_to = signal_data_[second_index + 1];
      float second_value = vital::utils::interpolate(second_from, second_to, second_t);
      total_to += second_value;

      float delta = first_value - second_value;
      error += delta * delta;
    }

    float total_diff = total_from - total_to;
    error += total_diff * total_diff * kDcDeltaErrorMultiplier;
  }

  return error;
}

float PitchDetector::findYinPeriod(int max_period) {
  // The method tries a range of candidate periods and selects the one with the smallest error.
  // It first does a coarse search, then refines around the best candidate to find a more exact period.
  constexpr float kMinLength = 300.0f;

  float max_length = std::min<float>(size_ / 2.0f, max_period);

  float best_error = INT_MAX;
  float match = kMinLength;

  for (float length = kMinLength; length < max_length; length += 1.0f) {
    float error = getPeriodError(length);
    if (error < best_error) {
      best_error = error;
      match = length;
    }
  }

  float best_match = match;
  for (float length = match - 1.0f; length <= match + 1.0f; length += 0.1f) {
    float error = getPeriodError(length);
    if (error < best_error) {
      best_error = error;
      best_match = length;
    }
  }

  return best_match;
}

float PitchDetector::matchPeriod(int max_period) {
  // A simple wrapper calling findYinPeriod to find the period.
  return findYinPeriod(max_period);
}
