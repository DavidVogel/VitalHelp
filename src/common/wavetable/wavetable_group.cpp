/*
Summary:
WavetableGroup manages a collection of WavetableComponents, each potentially generating or modifying waveforms. It allows adding, removing, and reordering components, provides methods to render their combined output at any position in the wavetable, and handles serialization. By grouping components, it supports layering and complex combinations that form part of a larger WavetableCreator pipeline.
 */

#include "wavetable_group.h"
#include "synth_constants.h"
#include "wave_frame.h"
#include "wave_source.h"
#include "wavetable.h"

int WavetableGroup::getComponentIndex(WavetableComponent* component) {
  for (int i = 0; i < components_.size(); ++i) {
    if (components_[i].get() == component)
      return i;
  }
  return -1;
}

void WavetableGroup::moveUp(int index) {
  // Swap the component at index with the one above it, if possible.
  if (index <= 0)
    return;

  components_[index].swap(components_[index - 1]);
}

void WavetableGroup::moveDown(int index) {
  // Swap the component at index with the one below it, if possible.
  if (index < 0 || index >= components_.size() - 1)
    return;

  components_[index].swap(components_[index + 1]);
}

void WavetableGroup::removeComponent(int index) {
  if (index < 0 || index >= components_.size())
    return;

  std::unique_ptr<WavetableComponent> component = std::move(components_[index]);
  components_.erase(components_.begin() + index);
}

void WavetableGroup::reset() {
  // Clear all components and load a default configuration.
  components_.clear();
  loadDefaultGroup();
}

void WavetableGroup::prerender() {
  // Allow each component to do any precomputation before rendering.
  for (auto& component : components_)
    component->prerender();
}

bool WavetableGroup::isShepardTone() {
  // Checks if all components are Shepard tone sources.
  for (auto& component : components_) {
    if (component->getType() != WavetableComponentFactory::kShepardToneSource)
      return false;
  }

  return true;
}

void WavetableGroup::render(vital::WaveFrame* wave_frame, float position) const {
  // Render all components and combine their outputs into wave_frame.
  wave_frame->index = position;

  for (auto& component : components_)
    component->render(wave_frame, position);
}

void WavetableGroup::renderTo(vital::Wavetable* wavetable) {
  // Fill the wavetable by rendering each frame from 0 to kNumOscillatorWaveFrames - 1.
  for (int i = 0; i < vital::kNumOscillatorWaveFrames; ++i) {
    compute_frame_.index = i;

    for (auto& component : components_)
      component->render(&compute_frame_, i);

    wavetable->loadWaveFrame(&compute_frame_);
  }
}

void WavetableGroup::loadDefaultGroup() {
  // Create a simple default group with a basic wave source that produces a linear ramp.
  WaveSource* wave_source = new WaveSource();
  wave_source->insertNewKeyframe(0);
  vital::WaveFrame* wave_frame = wave_source->getWaveFrame(0);
  for (int i = 0; i < vital::WaveFrame::kWaveformSize; ++i) {
    float t = i / (vital::WaveFrame::kWaveformSize - 1.0f);
    int half_shift = (i + vital::WaveFrame::kWaveformSize / 2) % vital::WaveFrame::kWaveformSize;
    wave_frame->time_domain[half_shift] = 1.0f - 2.0f * t;
  }
  wave_frame->toFrequencyDomain();

  addComponent(wave_source);
}

int WavetableGroup::getLastKeyframePosition() {
  // Find the maximum keyframe position among all components in this group.
  int last_position = 0;
  for (auto& component : components_)
    last_position = std::max(last_position, component->getLastKeyframePosition());

  return last_position;
}

json WavetableGroup::stateToJson() {
  // Serialize all components in the group.
  json json_components;
  for (auto& component : components_) {
    json json_component = component->stateToJson();
    json_components.push_back(json_component);
  }

  return { { "components", json_components } };
}

void WavetableGroup::jsonToState(json data) {
  // Clear existing components and restore from JSON data.
  components_.clear();

  json json_components = data["components"];
  for (json json_component : json_components) {
    std::string type = json_component["type"];
    WavetableComponent* component = WavetableComponentFactory::createComponent(type);
    component->jsonToState(json_component);
    addComponent(component);
  }
}
