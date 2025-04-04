/*
Summary:
WaveFoldModifier applies a nonlinear wave-folding effect to a wavetable’s time-domain waveform. By scaling and folding the waveform through sine and arcsine functions, it adds harmonic complexity and can create interesting timbral variations. Adjusting the fold boost parameter in keyframes and interpolating between them allows dynamic control over the amount of wave folding across the wavetable.
 */

#include "wave_fold_modifier.h"

#include "utils.h"
#include "wave_frame.h"

WaveFoldModifier::WaveFoldModifierKeyframe::WaveFoldModifierKeyframe() {
  // Default fold boost set to 1.0 for neutral folding.
  wave_fold_boost_ = 1.0f;
}

void WaveFoldModifier::WaveFoldModifierKeyframe::copy(const WavetableKeyframe* keyframe) {
  const WaveFoldModifierKeyframe* source = dynamic_cast<const WaveFoldModifierKeyframe*>(keyframe);
  wave_fold_boost_ = source->wave_fold_boost_;
}

void WaveFoldModifier::WaveFoldModifierKeyframe::interpolate(const WavetableKeyframe* from_keyframe,
                                                             const WavetableKeyframe* to_keyframe,
                                                             float t) {
  const WaveFoldModifierKeyframe* from = dynamic_cast<const WaveFoldModifierKeyframe*>(from_keyframe);
  const WaveFoldModifierKeyframe* to = dynamic_cast<const WaveFoldModifierKeyframe*>(to_keyframe);

  // Linearly interpolate the fold boost between two keyframes.
  wave_fold_boost_ = linearTween(from->wave_fold_boost_, to->wave_fold_boost_, t);
}

void WaveFoldModifier::WaveFoldModifierKeyframe::render(vital::WaveFrame* wave_frame) {
  // Wave-folding is achieved by scaling and mapping the waveform’s amplitude through a nonlinear transform.
  float max_value = std::max(1.0f, wave_frame->getMaxZeroOffset());

  for (int i = 0; i < vital::WaveFrame::kWaveformSize; ++i) {
    // Clamp the waveform's value to [-1, 1] relative to max_value, then map through asin and sin to fold it.
    float value = vital::utils::clamp(wave_frame->time_domain[i] / max_value, -1.0f, 1.0f);
    float adjusted_value = max_value * wave_fold_boost_ * asinf(value);

    wave_frame->time_domain[i] = sinf(adjusted_value);
  }
  wave_frame->toFrequencyDomain();
}

json WaveFoldModifier::WaveFoldModifierKeyframe::stateToJson() {
  json data = WavetableKeyframe::stateToJson();
  data["fold_boost"] = wave_fold_boost_;
  return data;
}

void WaveFoldModifier::WaveFoldModifierKeyframe::jsonToState(json data) {
  WavetableKeyframe::jsonToState(data);
  wave_fold_boost_ = data["fold_boost"];
}

WavetableKeyframe* WaveFoldModifier::createKeyframe(int position) {
  // Interpolate the fold boost parameter from the component’s keyframe list at the given position.
  WaveFoldModifierKeyframe* keyframe = new WaveFoldModifierKeyframe();
  interpolate(keyframe, position);
  return keyframe;
}

void WaveFoldModifier::render(vital::WaveFrame* wave_frame, float position) {
  // Interpolate parameters for this position and apply the folding transformation.
  interpolate(&compute_frame_, position);
  compute_frame_.render(wave_frame);
}

WavetableComponentFactory::ComponentType WaveFoldModifier::getType() {
  return WavetableComponentFactory::kWaveFolder;
}

WaveFoldModifier::WaveFoldModifierKeyframe* WaveFoldModifier::getKeyframe(int index) {
  WavetableKeyframe* wavetable_keyframe = keyframes_[index].get();
  return dynamic_cast<WaveFoldModifier::WaveFoldModifierKeyframe*>(wavetable_keyframe);
}
