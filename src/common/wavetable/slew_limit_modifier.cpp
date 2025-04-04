/*
Summary:
SlewLimitModifier applies slew-rate limiting to the wavetable’s time-domain signal, controlling how quickly the waveform can rise or fall per sample. By adjusting the upward and downward slew parameters, one can achieve smoother transitions and reduce harsh dynamics in the resulting sound. Interpolating between keyframes allows these constraints to evolve across the wavetable.
 */

#include "slew_limit_modifier.h"

#include "wave_frame.h"

SlewLimitModifier::SlewLimitModifierKeyframe::SlewLimitModifierKeyframe() {
  // Initialize upward and downward slew limits to zero, meaning no slew limiting.
  slew_down_run_rise_ = 0.0f;
  slew_up_run_rise_ = 0.0f;
}

void SlewLimitModifier::SlewLimitModifierKeyframe::copy(const WavetableKeyframe* keyframe) {
  const SlewLimitModifierKeyframe* source = dynamic_cast<const SlewLimitModifierKeyframe*>(keyframe);
  slew_down_run_rise_ = source->slew_down_run_rise_;
  slew_up_run_rise_ = source->slew_up_run_rise_;
}

void SlewLimitModifier::SlewLimitModifierKeyframe::interpolate(const WavetableKeyframe* from_keyframe,
                                                               const WavetableKeyframe* to_keyframe,
                                                               float t) {
  // Linearly interpolate upward and downward slew limits.
  const SlewLimitModifierKeyframe* from = dynamic_cast<const SlewLimitModifierKeyframe*>(from_keyframe);
  const SlewLimitModifierKeyframe* to = dynamic_cast<const SlewLimitModifierKeyframe*>(to_keyframe);

  slew_down_run_rise_ = linearTween(from->slew_down_run_rise_, to->slew_down_run_rise_, t);
  slew_up_run_rise_ = linearTween(from->slew_up_run_rise_, to->slew_up_run_rise_, t);
}

void SlewLimitModifier::SlewLimitModifierKeyframe::render(vital::WaveFrame* wave_frame) {
  // Enforce slew limits in time domain by constraining how much the waveform can rise or fall per sample.
  float min_slew_limit = 1.0f / vital::WaveFrame::kWaveformSize;
  float max_up_delta = (2.0f / vital::WaveFrame::kWaveformSize) / std::max(slew_up_run_rise_, min_slew_limit);
  float max_down_delta = (2.0f / vital::WaveFrame::kWaveformSize) / std::max(slew_down_run_rise_, min_slew_limit);

  float current_value = wave_frame->time_domain[0];
  // Iterate through the waveform's samples, limiting upward and downward changes.
  for (int i = 1; i < 2 * vital::WaveFrame::kWaveformSize; ++i) {
    int index = i % vital::WaveFrame::kWaveformSize;
    float target_value = wave_frame->time_domain[index];
    float delta = target_value - current_value;

    if (delta > 0.0f)
      current_value += std::min(delta, max_up_delta);
    else
      current_value -= std::min(-delta, max_down_delta);

    wave_frame->time_domain[index] = current_value;
  }
  wave_frame->toFrequencyDomain();
}

json SlewLimitModifier::SlewLimitModifierKeyframe::stateToJson() {
  json data = WavetableKeyframe::stateToJson();
  data["up_run_rise"] = slew_up_run_rise_;
  data["down_run_rise"] = slew_down_run_rise_;
  return data;
}

void SlewLimitModifier::SlewLimitModifierKeyframe::jsonToState(json data) {
  WavetableKeyframe::jsonToState(data);
  slew_up_run_rise_ = data["up_run_rise"];
  slew_down_run_rise_ = data["down_run_rise"];
}

WavetableKeyframe* SlewLimitModifier::createKeyframe(int position) {
  SlewLimitModifierKeyframe* keyframe = new SlewLimitModifierKeyframe();
  interpolate(keyframe, position);
  return keyframe;
}

void SlewLimitModifier::render(vital::WaveFrame* wave_frame, float position) {
  // Interpolate parameters for the given position and apply slew limiting.
  interpolate(&compute_frame_, position);
  compute_frame_.render(wave_frame);
}

WavetableComponentFactory::ComponentType SlewLimitModifier::getType() {
  return WavetableComponentFactory::kSlewLimiter;
}

SlewLimitModifier::SlewLimitModifierKeyframe* SlewLimitModifier::getKeyframe(int index) {
  WavetableKeyframe* wavetable_keyframe = keyframes_[index].get();
  return dynamic_cast<SlewLimitModifier::SlewLimitModifierKeyframe*>(wavetable_keyframe);
}
