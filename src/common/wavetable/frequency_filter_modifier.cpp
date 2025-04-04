/*
Summary:
FrequencyFilterModifier is a wavetable component that modifies the harmonic content of a wave by applying frequency-domain filters like low-pass, band-pass, high-pass, or comb filtering. FrequencyFilterModifierKeyframe stores parameters (cutoff, shape) that define how frequencies are allowed or attenuated. By interpolating between keyframes and optionally normalizing the result, this component enables flexible and expressive frequency shaping of wavetables.
 */

#include "frequency_filter_modifier.h"

#include "futils.h"
#include "utils.h"
#include "wave_frame.h"

namespace {
  // Internal constants and helper functions for shaping the filter response.
  constexpr float kMinPower = -9.0f;
  constexpr float kMaxPower = 9.0f;
  constexpr int kMaxSlopeReach = 128;

  force_inline double powerScale(double value, double power) {
    // Applies a nonlinear scaling function to 'value', controlled by 'power'.
    // Used to shape the comb filter pattern or other transformations.
    static constexpr float kMinPower = 0.01f;
    if (fabs(power) < kMinPower)
      return value;

    double abs_value = fabs(value);

    double numerator = exp(power * abs_value) - 1.0f;
    double denominator = exp(power) - 1.0f;
    if (value >= 0.0f)
      return numerator / denominator;
    return -numerator / denominator;
  }

  force_inline float combWave(float t, float power) {
    // Generates a comb-like pattern based on the input time 't' and 'power'.
    // This pattern repeats and creates a unique frequency-domain shape for the comb filter mode.
    float range = t - floorf(t);
    return 2.0f * powerScale(1.0f - fabsf(2.0f * range - 1.0f), power);
  }
} // namespace

FrequencyFilterModifier::FrequencyFilterModifierKeyframe::FrequencyFilterModifierKeyframe() {
  cutoff_ = 4.0f;   // A default cutoff in log-space, representing a certain frequency bin.
  shape_ = 0.5f;    // Midway shape for the filter slope.
  style_ = kLowPass;
  normalize_ = true;
}

void FrequencyFilterModifier::FrequencyFilterModifierKeyframe::copy(const WavetableKeyframe* keyframe) {
  const FrequencyFilterModifierKeyframe* source = dynamic_cast<const FrequencyFilterModifierKeyframe*>(keyframe);
  shape_ = source->shape_;
  cutoff_ = source->cutoff_;
}

void FrequencyFilterModifier::FrequencyFilterModifierKeyframe::interpolate(const WavetableKeyframe* from_keyframe,
                                                                           const WavetableKeyframe* to_keyframe,
                                                                           float t) {
  const FrequencyFilterModifierKeyframe* from = dynamic_cast<const FrequencyFilterModifierKeyframe*>(from_keyframe);
  const FrequencyFilterModifierKeyframe* to = dynamic_cast<const FrequencyFilterModifierKeyframe*>(to_keyframe);

  // Linearly interpolate cutoff and shape between two keyframes.
  shape_ = linearTween(from->shape_, to->shape_, t);
  cutoff_ = linearTween(from->cutoff_, to->cutoff_, t);
}

void FrequencyFilterModifier::FrequencyFilterModifierKeyframe::render(vital::WaveFrame* wave_frame) {
  // Apply the multiplier to each frequency bin in the wave_frame's frequency domain representation.
  for (int i = 0; i < vital::WaveFrame::kNumRealComplex; ++i)
    wave_frame->frequency_domain[i] *= getMultiplier(i);

  wave_frame->toTimeDomain();

  // Optionally normalize the wave after applying the filter.
  if (normalize_) {
    wave_frame->normalize(true);
    wave_frame->toFrequencyDomain();
  }
}

json FrequencyFilterModifier::FrequencyFilterModifierKeyframe::stateToJson() {
  json data = WavetableKeyframe::stateToJson();
  data["cutoff"] = cutoff_;
  data["shape"] = shape_;
  return data;
}

void FrequencyFilterModifier::FrequencyFilterModifierKeyframe::jsonToState(json data) {
  WavetableKeyframe::jsonToState(data);
  cutoff_ = data["cutoff"];
  shape_ = data["shape"];
}

float FrequencyFilterModifier::FrequencyFilterModifierKeyframe::getMultiplier(float index) {
  // Convert cutoff parameter to a frequency index and then shape the response.
  float cutoff_index = std::pow(2.0f, cutoff_);
  float cutoff_delta = index - cutoff_index;

  float slope = 1.0f / vital::utils::interpolate(1.0f, kMaxSlopeReach, shape_ * shape_);
  float power = vital::utils::interpolate(kMinPower, kMaxPower, shape_);

  // Based on style, determine how frequencies above/below cutoff are attenuated.
  if (style_ == kLowPass)
    return vital::utils::clamp(1.0f - slope * cutoff_delta, 0.0f, 1.0f);
  if (style_ == kBandPass)
    return vital::utils::clamp(1.0f - fabsf(slope * cutoff_delta), 0.0f, 1.0f);
  if (style_ == kHighPass)
    return vital::utils::clamp(1.0f + slope * cutoff_delta, 0.0f, 1.0f);
  if (style_ == kComb)
    return combWave(index / (cutoff_index * 2.0f), power);

  return 0.0f;
}

WavetableKeyframe* FrequencyFilterModifier::createKeyframe(int position) {
  FrequencyFilterModifierKeyframe* keyframe = new FrequencyFilterModifierKeyframe();
  interpolate(keyframe, position);
  return keyframe;
}

void FrequencyFilterModifier::render(vital::WaveFrame* wave_frame, float position) {
  // Interpolate parameters for the given position, then render using those parameters.
  interpolate(&compute_frame_, position);
  compute_frame_.setStyle(style_);
  compute_frame_.setNormalize(normalize_);
  compute_frame_.render(wave_frame);
}

WavetableComponentFactory::ComponentType FrequencyFilterModifier::getType() {
  return WavetableComponentFactory::kFrequencyFilter;
}

json FrequencyFilterModifier::stateToJson() {
  // Serialize the current filter style and normalization setting.
  json data = WavetableComponent::stateToJson();
  data["style"] = style_;
  data["normalize"] = normalize_;
  return data;
}

void FrequencyFilterModifier::jsonToState(json data) {
  WavetableComponent::jsonToState(data);
  style_ = data["style"];
  normalize_ = data["normalize"];
}

FrequencyFilterModifier::FrequencyFilterModifierKeyframe* FrequencyFilterModifier::getKeyframe(int index) {
  WavetableKeyframe* wavetable_keyframe = keyframes_[index].get();
  return dynamic_cast<FrequencyFilterModifier::FrequencyFilterModifierKeyframe*>(wavetable_keyframe);
}
