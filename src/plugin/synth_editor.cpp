/*
Summary:

SynthEditor is the main UI class for a synth plugin. It displays a FullInterface GUI, applies scaling, ensures an aspect ratio, and updates the UI in response to plugin state changes. It manages reading user preferences (e.g., animation, window size) and resizing behavior.
*/
#include "synth_editor.h"

#include "authentication.h"
#include "default_look_and_feel.h"
#include "synth_plugin.h"
#include "load_save.h"

SynthEditor::SynthEditor(SynthPlugin& synth) :
    AudioProcessorEditor(&synth), SynthGuiInterface(&synth), synth_(synth), was_animating_(true) {
  static constexpr int kHeightBuffer = 50;

  // Set the global look and feel to the DefaultLookAndFeel.
  setLookAndFeel(DefaultLookAndFeel::instance());

  // Initialize authentication and reset the GUI.
  Authentication::create();
  gui_->reset();

  // Configure oscilloscope and audio memory for the GUI from the synth.
  gui_->setOscilloscopeMemory(synth.getOscilloscopeMemory());
  gui_->setAudioMemory(synth.getAudioMemory());

  // Enable or disable animation based on user settings.
  gui_->animate(LoadSave::shouldAnimateWidgets());

  // Set minimum window constraints and maintain aspect ratio.
  constrainer_.setMinimumSize(vital::kMinWindowWidth, vital::kMinWindowHeight);
  double ratio = (1.0 * vital::kDefaultWindowWidth) / vital::kDefaultWindowHeight;
  constrainer_.setFixedAspectRatio(ratio);
  constrainer_.setGui(gui_.get());
  setConstrainer(&constrainer_);

  // Ensure the window fits within the screen bounds.
  Rectangle<int> total_bounds = Desktop::getInstance().getDisplays().getTotalBounds(true);
  total_bounds.removeFromBottom(kHeightBuffer);

  // Make the GUI visible and size the window.
  addAndMakeVisible(gui_.get());
  float window_size = LoadSave::loadWindowSize();
  window_size = std::min(window_size, total_bounds.getWidth() / (1.0f * vital::kDefaultWindowWidth));
  window_size = std::min(window_size, total_bounds.getHeight() / (1.0f * vital::kDefaultWindowHeight));
  int width = std::round(window_size * vital::kDefaultWindowWidth);
  int height = std::round(window_size * vital::kDefaultWindowHeight);
  setResizable(true, true);
  setSize(width, height);
}

void SynthEditor::resized() {
  // Resize the GUI to fill the entire editor area.
  AudioProcessorEditor::resized();
  gui_->setBounds(getLocalBounds());
}

void SynthEditor::setScaleFactor(float newScale) {
  // Apply new scale factor and re-render the GUI background.
  AudioProcessorEditor::setScaleFactor(newScale);
  gui_->redoBackground();
}

void SynthEditor::updateFullGui() {
  // Trigger a full GUI update and notify the host that the UI might have changed.
  SynthGuiInterface::updateFullGui();
  synth_.updateHostDisplay();
}
