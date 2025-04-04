/*
Summary:
WaveWindowModifier applies a chosen windowing function to the beginning and end of a waveform, tapering its amplitude based on left_position_ and right_position_. Different window shapes (cosine, half-sine, linear, square, wiggle) produce various slopes and transitions. By interpolating between keyframes, users can animate the window size and shape over the wavetable, influencing how the waveform’s amplitude envelope evolves dynamically.
 */

#include "wave_frame.h"

#include "wave_window_modifier.h"

float WaveWindowModifier::applyWindow(WindowShape window_shape, float t) {
  // Applies the selected window function at time t.
  if (window_shape == kCos)
    return 0.5f - 0.5f * cosf(vital::kPi * t);
  if (window_shape == kHalfSin)
    return sinf(vital::kPi * t / 2.0f);
  if (window_shape == kSquare)
    return t < 1.0f ? 0.0f : 1.0f;
  if (window_shape == kWiggle)
    return t * cosf(vital::kPi * (t * 1.5f + 0.5f));
  // Default to linear if no shape matches.
  return t;
}

WaveWindowModifier::WaveWindowModifierKeyframe::WaveWindowModifierKeyframe() {
  // Default positions leave a portion of the wave fully amplitude-visible.
  static constexpr float kDefaultOffset = 0.25f;
  left_position_ = kDefaultOffset;
  right_position_ = 1.0f - kDefaultOffset;
  window_shape_ = kCos;
}

void WaveWindowModifier::WaveWindowModifierKeyframe::copy(const WavetableKeyframe* keyframe) {
  const WaveWindowModifierKeyframe* source = dynamic_cast<const WaveWindowModifierKeyframe*>(keyframe);

  left_position_ = source->left_position_;
  right_position_ = source->right_position_;
}

void WaveWindowModifier::WaveWindowModifierKeyframe::interpolate(const WavetableKeyframe* from_keyframe,
                                                                 const WavetableKeyframe* to_keyframe,
                                                                 float t) {
  const WaveWindowModifierKeyframe* from = dynamic_cast<const WaveWindowModifierKeyframe*>(from_keyframe);
  const WaveWindowModifierKeyframe* to = dynamic_cast<const WaveWindowModifierKeyframe*>(to_keyframe);

  left_position_ = linearTween(from->left_position_, to->left_position_, t);
  right_position_ = linearTween(from->right_position_, to->right_position_, t);
}

void WaveWindowModifier::WaveWindowModifierKeyframe::render(vital::WaveFrame* wave_frame) {
  // Apply the window shape at the start (up to left_position_) and end (after right_position_) of the wave.
  for (int i = 0; i < vital::WaveFrame::kWaveformSize; ++i) {
    float t = i / (vital::WaveFrame::kWaveformSize - 1.0f);
    if (t >= left_position_)
      break;

    wave_frame->time_domain[i] *= applyWindow(t / left_position_);
  }

  // Process from the end backwards until we hit right_position_.
  for (int i = vital::WaveFrame::kWaveformSize; i >= 0; --i) {
    float t = i / (vital::WaveFrame::kWaveformSize - 1.0f);
    if (t <= right_position_)
      break;

    wave_frame->time_domain[i] *= applyWindow((1.0f - t) / (1.0f - right_position_));
  }

  wave_frame->toFrequencyDomain();
}

json WaveWindowModifier::WaveWindowModifierKeyframe::stateToJson() {
  json data = WavetableKeyframe::stateToJson();
  data["left_position"] = left_position_;
  data["right_position"] = right_position_;
  return data;
}

void WaveWindowModifier::WaveWindowModifierKeyframe::jsonToState(json data) {
  WavetableKeyframe::jsonToState(data);
  left_position_ = data["left_position"];
  right_position_ = data["right_position"];
}

WavetableKeyframe* WaveWindowModifier::createKeyframe(int position) {
  WaveWindowModifierKeyframe* keyframe = new WaveWindowModifierKeyframe();
  interpolate(keyframe, position);
  return keyframe;
}

void WaveWindowModifier::render(vital::WaveFrame* wave_frame, float position) {
  // Interpolate parameters and apply the chosen window shape before rendering.
  interpolate(&compute_frame_, position);
  compute_frame_.setWindowShape(window_shape_);
  compute_frame_.render(wave_frame);
}

WavetableComponentFactory::ComponentType WaveWindowModifier::getType() {
  return WavetableComponentFactory::kWaveWindow;
}

json WaveWindowModifier::stateToJson() {
  json data = WavetableComponent::stateToJson();
  data["window_shape"] = window_shape_;
  return data;
}

void WaveWindowModifier::jsonToState(json data) {
  WavetableComponent::jsonToState(data);
  window_shape_ = data["window_shape"];
}

WaveWindowModifier::WaveWindowModifierKeyframe* WaveWindowModifier::getKeyframe(int index) {
  WavetableKeyframe* wavetable_keyframe = keyframes_[index].get();
  return dynamic_cast<WaveWindowModifier::WaveWindowModifierKeyframe*>(wavetable_keyframe);
}
