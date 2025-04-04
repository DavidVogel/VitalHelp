/*
Summary:
WaveSource and WaveSourceKeyframe provide a direct representation of wavetables via WaveFrames. By storing entire waveforms and supporting time or frequency domain interpolation (including cubic methods for smoothness), they enable flexible and sophisticated transitions between keyframes. This results in dynamic and evolving sounds that can be tailored by controlling interpolation modes and parameters.
 */

#include "wave_source.h"
#include "wave_frame.h"
#include "wavetable_component_factory.h"

WaveSource::WaveSource() {
  compute_frame_ = std::make_unique<WaveSourceKeyframe>();
  interpolation_mode_ = kFrequency; // Default to frequency interpolation.
}

WaveSource::~WaveSource() { }

WavetableKeyframe* WaveSource::createKeyframe(int position) {
  WaveSourceKeyframe* keyframe = new WaveSourceKeyframe();
  // Renders the initial WaveFrame at this position into the keyframe's frame.
  render(keyframe->wave_frame(), position);
  return keyframe;
}

void WaveSource::render(vital::WaveFrame* wave_frame, float position) {
  // Set the interpolation mode for the compute_frame_ and interpolate into it.
  compute_frame_->setInterpolationMode(interpolation_mode_);
  interpolate(compute_frame_.get(), position);
  wave_frame->copy(compute_frame_->wave_frame());
}

WavetableComponentFactory::ComponentType WaveSource::getType() {
  return WavetableComponentFactory::kWaveSource;
}

json WaveSource::stateToJson() {
  json data = WavetableComponent::stateToJson();
  data["interpolation"] = interpolation_mode_;
  return data;
}

void WaveSource::jsonToState(json data) {
  WavetableComponent::jsonToState(data);
  interpolation_mode_ = data["interpolation"];
  compute_frame_->setInterpolationMode(interpolation_mode_);
}

vital::WaveFrame* WaveSource::getWaveFrame(int index) {
  return getKeyframe(index)->wave_frame();
}

WaveSourceKeyframe* WaveSource::getKeyframe(int index) {
  WavetableKeyframe* wavetable_keyframe = keyframes_[index].get();
  return dynamic_cast<WaveSourceKeyframe*>(wavetable_keyframe);
}

void WaveSourceKeyframe::copy(const WavetableKeyframe* keyframe) {
  // Copies the WaveFrame data from another WaveSourceKeyframe.
  const WaveSourceKeyframe* source = dynamic_cast<const WaveSourceKeyframe*>(keyframe);
  wave_frame_->copy(source->wave_frame_.get());
}

void WaveSourceKeyframe::linearTimeInterpolate(const vital::WaveFrame* from, const vital::WaveFrame* to, float t) {
  // Perform a simple linear interpolation of time-domain samples between two WaveFrames.
  for (int i = 0; i < vital::WaveFrame::kWaveformSize; ++i)
    wave_frame_->time_domain[i] = linearTween(from->time_domain[i], to->time_domain[i], t);
  wave_frame_->toFrequencyDomain();
}

void WaveSourceKeyframe::cubicTimeInterpolate(const vital::WaveFrame* prev, const vital::WaveFrame* from,
                                              const vital::WaveFrame* to, const vital::WaveFrame* next,
                                              float range_prev, float range, float range_next, float t) {

  // Cubic interpolation for an even smoother transition in the time domain.
  for (int i = 0; i < vital::WaveFrame::kWaveformSize; ++i) {
    wave_frame_->time_domain[i] = cubicTween(prev->time_domain[i], from->time_domain[i],
                                             to->time_domain[i], next->time_domain[i],
                                             range_prev, range, range_next, t);
  }
  wave_frame_->toFrequencyDomain();
}

void WaveSourceKeyframe::linearFrequencyInterpolate(const vital::WaveFrame* from,
                                                    const vital::WaveFrame* to, float t) {
  // Linear interpolation in frequency domain: smoothly blend magnitudes and phases of harmonics.
  for (int i = 0; i < vital::WaveFrame::kNumRealComplex; ++i) {
    float amplitude_from = sqrtf(std::abs(from->frequency_domain[i]));
    float amplitude_to = sqrtf(std::abs(to->frequency_domain[i]));
    float amplitude = linearTween(amplitude_from, amplitude_to, t);
    amplitude *= amplitude;

    // Interpolate DC and last harmonic separately for consistency.
    float phase_from = std::arg(from->frequency_domain[i]);
    float phase_delta = std::arg(std::conj(from->frequency_domain[i]) * to->frequency_domain[i]);
    float phase = phase_from + t * phase_delta;
    if (amplitude_from == 0)
      phase = std::arg(to->frequency_domain[i]);
    wave_frame_->frequency_domain[i] = std::polar(amplitude, phase);
  }

  float dc_from = from->frequency_domain[0].real();
  float dc_to = to->frequency_domain[0].real();
  wave_frame_->frequency_domain[0] = linearTween(dc_from, dc_to, t);

  int last = vital::WaveFrame::kNumRealComplex - 1;
  float last_harmonic_from = from->frequency_domain[last].real();
  float last_harmonic_to = to->frequency_domain[last].real();
  wave_frame_->frequency_domain[last] = linearTween(last_harmonic_from, last_harmonic_to, t);

  wave_frame_->toTimeDomain();
}

void WaveSourceKeyframe::cubicFrequencyInterpolate(const vital::WaveFrame* prev, const vital::WaveFrame* from,
                                                   const vital::WaveFrame* to, const vital::WaveFrame* next,
                                                   float range_prev, float range, float range_next, float t) {
  // Cubic interpolation in frequency domain for very smooth harmonic transitions across multiple frames.
  for (int i = 0; i < vital::WaveFrame::kNumRealComplex; ++i) {
    float amplitude_prev = sqrtf(std::abs(prev->frequency_domain[i]));
    float amplitude_from = sqrtf(std::abs(from->frequency_domain[i]));
    float amplitude_to = sqrtf(std::abs(to->frequency_domain[i]));
    float amplitude_next = sqrtf(std::abs(next->frequency_domain[i]));
    float amplitude = cubicTween(amplitude_prev, amplitude_from, amplitude_to, amplitude_next,
                                 range_prev, range, range_next, t);
    amplitude *= amplitude;

    // Calculate phase transitions similarly, ensuring continuous phase evolution.
    float phase_delta_from = std::arg(std::conj(prev->frequency_domain[i]) * from->frequency_domain[i]);
    float phase_delta_to = std::arg(std::conj(from->frequency_domain[i]) * to->frequency_domain[i]);
    float phase_delta_next = std::arg(std::conj(to->frequency_domain[i]) * next->frequency_domain[i]);
    float phase_prev = std::arg(prev->frequency_domain[i]);
    float phase_from = phase_prev;
    if (amplitude_from)
      phase_from += phase_delta_from;
    float phase_to = phase_from;
    if (amplitude_to)
      phase_to += phase_delta_to;
    float phase_next = phase_to;
    if (amplitude_next)
      phase_next += phase_delta_next;

    float phase = cubicTween(phase_prev, phase_from, phase_to, phase_next, range_prev, range, range_next, t);
    wave_frame_->frequency_domain[i] = std::polar(amplitude, phase);
  }

  // Handle DC and last harmonic separately with cubic interpolation.
  float dc_prev = prev->frequency_domain[0].real();
  float dc_from = from->frequency_domain[0].real();
  float dc_to = to->frequency_domain[0].real();
  float dc_next = next->frequency_domain[0].real();
  wave_frame_->frequency_domain[0] = cubicTween(dc_prev, dc_from, dc_to, dc_next, range_prev, range, range_next, t);

  int last = vital::WaveFrame::kNumRealComplex - 1;
  float last_harmonic_prev = prev->frequency_domain[last].real();
  float last_harmonic_from = from->frequency_domain[last].real();
  float last_harmonic_to = to->frequency_domain[last].real();
  float last_harmonic_next = next->frequency_domain[last].real();
  wave_frame_->frequency_domain[last] = cubicTween(last_harmonic_prev, last_harmonic_from,
                                                   last_harmonic_to, last_harmonic_next,
                                                   range_prev, range, range_next, t);
  wave_frame_->toTimeDomain();
}

void WaveSourceKeyframe::interpolate(const WavetableKeyframe* from_keyframe,
                                     const WavetableKeyframe* to_keyframe, float t) {
  const WaveSourceKeyframe* from = dynamic_cast<const WaveSourceKeyframe*>(from_keyframe);
  const WaveSourceKeyframe* to = dynamic_cast<const WaveSourceKeyframe*>(to_keyframe);

  if (interpolation_mode_ == WaveSource::kFrequency)
    linearFrequencyInterpolate(from->wave_frame_.get(), to->wave_frame_.get(), t);
  else
    linearTimeInterpolate(from->wave_frame_.get(), to->wave_frame_.get(), t);
}

void WaveSourceKeyframe::smoothInterpolate(const WavetableKeyframe* prev_keyframe,
                                           const WavetableKeyframe* from_keyframe,
                                           const WavetableKeyframe* to_keyframe,
                                           const WavetableKeyframe* next_keyframe, float t) {
  // Uses cubic interpolation if possible for even smoother transitions.
  const vital::WaveFrame* prev = dynamic_cast<const WaveSourceKeyframe*>(prev_keyframe)->wave_frame_.get();
  const vital::WaveFrame* from = dynamic_cast<const WaveSourceKeyframe*>(from_keyframe)->wave_frame_.get();
  const vital::WaveFrame* to = dynamic_cast<const WaveSourceKeyframe*>(to_keyframe)->wave_frame_.get();
  const vital::WaveFrame* next = dynamic_cast<const WaveSourceKeyframe*>(next_keyframe)->wave_frame_.get();

  float range_prev = from_keyframe->position() - prev_keyframe->position();
  float range = to_keyframe->position() - from_keyframe->position();
  float range_next = next_keyframe->position() - to_keyframe->position();

  if (interpolation_mode_ == WaveSource::kFrequency)
    cubicFrequencyInterpolate(prev, from, to, next, range_prev, range, range_next, t);
  else
    cubicTimeInterpolate(prev, from, to, next, range_prev, range, range_next, t);
}

json WaveSourceKeyframe::stateToJson() {
  // Serialize the waveform’s time-domain data as Base64.
  String encoded = Base64::toBase64(wave_frame_->time_domain, sizeof(float) * vital::WaveFrame::kWaveformSize);
  json data = WavetableKeyframe::stateToJson();
  data["wave_data"] = encoded.toStdString();
  return data;
}

void WaveSourceKeyframe::jsonToState(json data) {
  WavetableKeyframe::jsonToState(data);

  MemoryOutputStream decoded(sizeof(float) * vital::WaveFrame::kWaveformSize);
  std::string wave_data = data["wave_data"];
  Base64::convertFromBase64(decoded, wave_data);
  memcpy(wave_frame_->time_domain, decoded.getData(), sizeof(float) * vital::WaveFrame::kWaveformSize);
  wave_frame_->toFrequencyDomain();
}
