/// @file sample_viewer.cpp
/// @brief Implements the SampleViewer class for displaying and animating a waveform sample.

#include "sample_viewer.h"

#include "skin.h"
#include "synth_gui_interface.h"

/// Constructs a new SampleViewer with default parameters.
SampleViewer::SampleViewer() : OpenGlLineRenderer(kResolution), bottom_(kResolution),
                               dragging_overlay_(Shaders::kColorFragment) {
  addAndMakeVisible(bottom_);
  animate_ = false;
  active_ = true;
  sample_phase_output_ = nullptr;
  last_phase_ = 0.0f;
  sample_ = nullptr;
  dragging_audio_file_ = false;
  addBottomRoundedCorners();

  dragging_overlay_.setTargetComponent(this);

  setFill(true);
  bottom_.setFill(true);
  setLineWidth(2.0f);
  bottom_.setLineWidth(2.0f);
}

/// Destructor.
SampleViewer::~SampleViewer() { }

/// Handles the loading of an audio file.
/// Notifies registered listeners and updates the waveform display.
/// @param file The audio file that was loaded.
void SampleViewer::audioFileLoaded(const File& file) {
  for (Listener* listener : listeners_)
    listener->sampleLoaded(file);

  setLinePositions();
}

/// Repaints the audio waveform by resetting drag state and repositioning the lines.
void SampleViewer::repaintAudio() {
  dragging_audio_file_ = false;
  setLinePositions();
}

/// Sets the waveform line positions based on the currently loaded sample.
void SampleViewer::setLinePositions() {
  if (sample_ == nullptr)
    return;

  double sample_length = sample_->originalLength();
  const vital::mono_float* buffer = sample_->buffer();
  float center = getHeight() / 2.0f;
  for (int i = 0; i < kResolution; ++i) {
    int start_index = std::min<int>(sample_length * i / kResolution, sample_length);
    int end_index = std::min<int>((sample_length * (i + 1) + kResolution - 1) / kResolution, sample_length);
    float max = buffer[start_index];
    for (int i = start_index + 1; i < end_index; ++i)
      max = std::max(max, buffer[i]);
    setYAt(i, center - max * center);
    bottom_.setYAt(i, center + max * center);
  }
}

/// Retrieves the name of the currently loaded sample.
/// @return The sample name, or an empty string if no sample is loaded.
std::string SampleViewer::getName() {
  if (sample_)
    return sample_->getName();
  return "";
}

/// Handles the component resize event.
/// Adjusts the positions of the waveform points and related components.
void SampleViewer::resized() {
  bottom_.setBounds(getLocalBounds());
  dragging_overlay_.setColor(findColour(Skin::kOverlayScreen, true));

  for (int i = 0; i < kResolution; ++i) {
    float t = i / (kResolution - 1.0f);
    setXAt(i, getWidth() * t);
    bottom_.setXAt(i, getWidth() * t);
  }

  if (sample_phase_output_ == nullptr) {
    SynthGuiInterface* parent = findParentComponentOfClass<SynthGuiInterface>();
    if (parent)
      sample_phase_output_ = parent->getSynth()->getStatusOutput("sample_phase");
  }

  OpenGlLineRenderer::resized();
  setLinePositions();
}

/// Renders the waveform and applies animations.
/// This is called regularly to update the displayed waveform state.
/// @param open_gl The OpenGlWrapper for managing OpenGL state.
/// @param animate If true, the component should animate its waveform.
void SampleViewer::render(OpenGlWrapper& open_gl, bool animate) {
  animate_ = animate;

  float boost_amount = findValue(Skin::kWidgetLineBoost);
  float fill_boost_amount = findValue(Skin::kWidgetFillBoost);
  setBoostAmount(boost_amount);
  bottom_.setBoostAmount(boost_amount);
  setFillBoostAmount(fill_boost_amount);
  bottom_.setFillBoostAmount(fill_boost_amount);

  if (sample_ == nullptr)
    return;

  int sample_length = sample_->originalLength();
  if (sample_phase_output_ == nullptr || sample_length == 0)
    return;

    // Determine if voice changed
  vital::poly_float encoded_phase = sample_phase_output_->value();
  std::pair<vital::poly_float, vital::poly_float> decoded = vital::utils::decodePhaseAndVoice(encoded_phase);
  vital::poly_float phase = decoded.first;
  vital::poly_float voice = decoded.second;

  vital::poly_mask switch_mask = vital::poly_float::notEqual(voice, last_voice_);
  vital::poly_float phase_reset = vital::utils::max(0.0f, phase);
  last_phase_ = vital::utils::maskLoad(last_phase_, phase_reset, switch_mask);

  if (!sample_phase_output_->isClearValue(phase) && vital::poly_float::notEqual(phase, 0.0f).anyMask() != 0) {
    vital::poly_float phase_delta = vital::poly_float::abs(phase - last_phase_);
    vital::poly_float decay = vital::poly_float(1.0f) - phase_delta * kSpeedDecayMult;
    decay = vital::utils::clamp(decay, kBoostDecay, 1.0f);
    decayBoosts(decay);
    bottom_.decayBoosts(decay);

    if (animate_) {
      boostRange(last_phase_, phase, 0, decay);
      bottom_.boostRange(last_phase_, phase, 0, decay);
    }
  }
  else {
    decayBoosts(kBoostDecay);
    bottom_.decayBoosts(kBoostDecay);
  }

  last_phase_ = phase;
  last_voice_ = voice;

  float fill_fade = 0.0f;
  if (parent_)
    fill_fade = parent_->findValue(Skin::kWidgetFillFade);

  // First pass for underlying lines
  Colour line, fill;
  if (isActive()) {
    line = findColour(Skin::kWidgetPrimary2, true);
    fill = findColour(Skin::kWidgetSecondary2, true);
  }
  else {
    line = findColour(Skin::kWidgetPrimaryDisabled, true);
    fill = findColour(Skin::kWidgetSecondaryDisabled, true);
  }

  setColor(line);
  bottom_.setColor(line);
  setFillColors(fill.withMultipliedAlpha(1.0f - fill_fade), fill);
  bottom_.setFillColors(fill.withMultipliedAlpha(1.0f - fill_fade), fill);

  drawLines(open_gl, false);
  bottom_.drawLines(open_gl, false);

  // Second pass for boosted lines
  if (isActive()) {
    line = findColour(Skin::kWidgetPrimary1, true);
    fill = findColour(Skin::kWidgetSecondary1, true);
  }
  else {
    line = findColour(Skin::kWidgetPrimaryDisabled, true);
    fill = findColour(Skin::kWidgetSecondaryDisabled, true);
  }

  setColor(line);
  bottom_.setColor(line);
  setFillColors(fill.withMultipliedAlpha(1.0f - fill_fade), fill);
  bottom_.setFillColors(fill.withMultipliedAlpha(1.0f - fill_fade), fill);

  drawLines(open_gl, anyBoostValue());
  bottom_.drawLines(open_gl, anyBoostValue());

  // Render overlay if dragging a file
  if (dragging_audio_file_)
    dragging_overlay_.render(open_gl, animate);
  renderCorners(open_gl, animate);
}
