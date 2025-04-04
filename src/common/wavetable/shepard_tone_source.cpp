/*
Summary:
ShepardToneSource uses a single keyframe and a special looping technique to produce a continuously rising (or falling) tone—an auditory illusion known as a Shepard tone. By rearranging frequency components into a loop frame and interpolating between these frames, this source can create a stable, looping sonic texture that simulates infinite pitch movement.
 */

#include "shepard_tone_source.h"
#include "wavetable_component_factory.h"

ShepardToneSource::ShepardToneSource() {
  // Create a loop frame to facilitate continuous looping of the frequency domain spectrum.
  loop_frame_ = std::make_unique<WaveSourceKeyframe>();
}

ShepardToneSource::~ShepardToneSource() { }

void ShepardToneSource::render(vital::WaveFrame* wave_frame, float position) {
  if (numFrames() == 0)
    return;

  // Retrieve the single keyframe that holds the base spectrum.
  WaveSourceKeyframe* keyframe = getKeyframe(0);
  vital::WaveFrame* key_wave_frame = keyframe->wave_frame();
  vital::WaveFrame* loop_wave_frame = loop_frame_->wave_frame();

  // Interleave frequency components in a pattern (e.g., placing them at every even index)
  // to produce a continuous looping effect in the frequency domain.
  for (int i = 0; i < vital::WaveFrame::kWaveformSize / 2; ++i) {
    loop_wave_frame->frequency_domain[i * 2] = key_wave_frame->frequency_domain[i];
    loop_wave_frame->frequency_domain[i * 2 + 1] = 0.0f;
  }

  loop_wave_frame->toTimeDomain();

  // Use compute_frame_ (inherited from WaveSource) to interpolate between keyframe and loop_frame_
  // based on the given position, producing a stable Shepard tone-like result.
  compute_frame_->setInterpolationMode(interpolation_mode_);
  compute_frame_->interpolate(keyframe, loop_frame_.get(), position / (vital::kNumOscillatorWaveFrames - 1.0f));
  wave_frame->copy(compute_frame_->wave_frame());
}

WavetableComponentFactory::ComponentType ShepardToneSource::getType() {
  return WavetableComponentFactory::kShepardToneSource;
}
