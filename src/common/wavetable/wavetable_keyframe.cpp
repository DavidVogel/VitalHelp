/*
Summary:
WavetableKeyframe defines a single point in a wavetable where the waveform configuration is known. By interpolating between keyframes, a component can produce evolving waveforms. This class provides generic interpolation (linear and cubic) and serialization methods, while subclasses specify how waveform data is stored and rendered.
 */

#include "wavetable_keyframe.h"

#include "utils.h"
#include "wavetable_component.h"

float WavetableKeyframe::linearTween(float point_from, float point_to, float t) {
  // Uses a simple linear interpolation from utils.
  return vital::utils::interpolate(point_from, point_to, t);
}

float WavetableKeyframe::cubicTween(float point_prev, float point_from, float point_to, float point_next,
                                    float range_prev, float range, float range_next, float t) {
  // Computes a cubic interpolation between four points for smoother transitions than linear.
  float slope_from = 0.0f;
  float slope_to = 0.0f;
  if (range_prev > 0.0f)
    slope_from = (point_to - point_prev) / (1.0f + range_prev / range);
  if (range_next > 0.0f)
    slope_to = (point_next - point_from) / (1.0f + range_next / range);
  float delta = point_to - point_from;

  float movement = linearTween(point_from, point_to, t);
  float smooth = t * (1.0f - t) * ((1.0f - t) * (slope_from - delta) + t * (delta - slope_to));
  return movement + smooth;
}

int WavetableKeyframe::index() {
  // Retrieves the index of this keyframe by asking the owner component.
  return owner()->indexOf(this);
}

json WavetableKeyframe::stateToJson() {
  return { { "position", position_ } };
}

void WavetableKeyframe::jsonToState(json data) {
  // Restore the position from the JSON data.
  position_ = data["position"];
}
