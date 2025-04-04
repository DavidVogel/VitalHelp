/*
Summary:
WavetableComponent is the base class for elements that produce or modify wavetables. It manages keyframes representing waveform states at certain positions, and supports interpolation between these states using none, linear, or cubic methods. By serializing and deserializing keyframes, it integrates smoothly with preset systems. Interpolation ensures smooth transitions across the wavetable dimension, enabling dynamic and evolving sounds.
 */

#include "wavetable_component.h"

WavetableKeyframe* WavetableComponent::insertNewKeyframe(int position) {
  VITAL_ASSERT(position >= 0 && position < vital::kNumOscillatorWaveFrames);

  // Create a new keyframe for the given position and insert it at the correct sorted position.
  WavetableKeyframe* keyframe = createKeyframe(position);
  keyframe->setOwner(this);
  keyframe->setPosition(position);

  int index = getIndexFromPosition(position);
  keyframes_.insert(keyframes_.begin() + index, std::unique_ptr<WavetableKeyframe>(keyframe));
  return keyframe;
}

void WavetableComponent::reposition(WavetableKeyframe* keyframe) {
  // Remove and re-insert a keyframe after its position changed to maintain sorted order.
  int start_index = indexOf(keyframe);
  keyframes_[start_index].release();
  keyframes_.erase(keyframes_.begin() + start_index);

  int new_index = getIndexFromPosition(keyframe->position());
  keyframes_.insert(keyframes_.begin() + new_index, std::unique_ptr<WavetableKeyframe>(keyframe));
}

void WavetableComponent::remove(WavetableKeyframe* keyframe) {
  // Erase a keyframe from the vector.
  int start_index = indexOf(keyframe);
  keyframes_.erase(keyframes_.begin() + start_index);
}

void WavetableComponent::jsonToState(json data) {
  // Clears current keyframes and load them from the JSON structure.
  keyframes_.clear();
  for (json json_keyframe : data["keyframes"]) {
    WavetableKeyframe* keyframe = insertNewKeyframe(json_keyframe["position"]);
    keyframe->jsonToState(json_keyframe);
  }

  if (data.count("interpolation_style"))
    interpolation_style_ = data["interpolation_style"];
}

json WavetableComponent::stateToJson() {
  // Serialize all keyframes and the component's interpolation style.
  json keyframes_data;
  for (int i = 0; i < keyframes_.size(); ++i)
    keyframes_data.emplace_back(keyframes_[i]->stateToJson());

  return {
    { "keyframes", keyframes_data },
    { "type", WavetableComponentFactory::getComponentName(getType()) },
    { "interpolation_style", interpolation_style_ }
  };
}

void WavetableComponent::reset() {
  // Clear all keyframes and add a default one at position 0.
  keyframes_.clear();
  insertNewKeyframe(0);
}

void WavetableComponent::interpolate(WavetableKeyframe* dest, float position) {
  // Given a position, find the appropriate keyframes to interpolate between.
  if (numFrames() == 0)
    return;

  int index = getIndexFromPosition(position) - 1;
  int clamped_index = std::min(std::max(index, 0), numFrames() - 1);
  WavetableKeyframe* from_frame = keyframes_[clamped_index].get();

  // Depending on the interpolation style, copy, linear interpolate, or cubic interpolate.
  if (index < 0 || index >= numFrames() - 1 || interpolation_style_ == kNone)
    dest->copy(from_frame);
  else if (interpolation_style_ == kLinear) {
    WavetableKeyframe* to_frame = keyframes_[index + 1].get();
    int from_position = keyframes_[index]->position();
    int to_position = keyframes_[index + 1]->position();
    float t = (1.0f * position - from_position) / (to_position - from_position);
    dest->interpolate(from_frame, to_frame, t);
  }
  else if (interpolation_style_ == kCubic) {
    // Cubic interpolation uses prev and next frames as well.
    int next_index = index + 2;
    int prev_index = index - 1;
    if (next_index >= numFrames())
      next_index = index;
    if (prev_index < 0)
      prev_index = index + 1;

    WavetableKeyframe* to_frame = keyframes_[index + 1].get();
    WavetableKeyframe* next_frame = keyframes_[next_index].get();
    WavetableKeyframe* prev_frame = keyframes_[prev_index].get();

    int from_position = keyframes_[index]->position();
    int to_position = keyframes_[index + 1]->position();
    float t = (1.0f * position - from_position) / (to_position - from_position);
    dest->smoothInterpolate(prev_frame, from_frame, to_frame, next_frame, t);
  }
}

int WavetableComponent::getIndexFromPosition(int position) const {
  // Returns the insertion index to keep keyframes sorted by their position.
  int index = 0;
  for (auto& keyframe : keyframes_) {
    if (position < keyframe->position())
      break;
    index++;
  }

  return index;
}

WavetableKeyframe* WavetableComponent::getFrameAtPosition(int position) {
  int index = getIndexFromPosition(position);
  if (index < 0 || index >= keyframes_.size())
    return nullptr;

  return keyframes_[index].get();
}

int WavetableComponent::getLastKeyframePosition() {
  if (keyframes_.size() == 0)
    return 0;
  if (!hasKeyframes())
    return vital::kNumOscillatorWaveFrames - 1;
  return keyframes_[keyframes_.size() - 1]->position();
}
