/// @file tempo_selector.cpp
/// @brief Implements the TempoSelector class, which provides different tempo-related modes and controls.

#include "tempo_selector.h"

#include "paths.h"
#include "skin.h"
#include "default_look_and_feel.h"

/// Constructs a TempoSelector and sets initial rendering mode.
TempoSelector::TempoSelector(String name) : SynthSlider(name), free_slider_(nullptr),
                                            tempo_slider_(nullptr), keytrack_transpose_slider_(nullptr),
                                            keytrack_tune_slider_(nullptr) {
  paintToImage(true);
}

/// Handles mouse-down events to show the mode selection popup if not a right-click context menu event.
/// @param e The mouse event.
void TempoSelector::mouseDown(const MouseEvent& e) {
  if (e.mods.isPopupMenu()) {
    SynthSlider::mouseDown(e);
    return;
  }

  PopupItems options;
  options.addItem(kSeconds, "Seconds");
  options.addItem(kTempo, "Tempo");
  options.addItem(kTempoDotted, "Tempo Dotted");
  options.addItem(kTempoTriplet, "Tempo Triplets");
  if (getMaximum() >= kKeytrack)
    options.addItem(kKeytrack, "Keytrack");

  parent_->showPopupSelector(this, Point<int>(0, getHeight()), options, [=](int value) { setValue(value); });
}

/// Handles mouse-up events to re-show the menu if right-clicked.
/// @param e The mouse event.
void TempoSelector::mouseUp(const MouseEvent& e) {
  if (e.mods.isPopupMenu()) {
    SynthSlider::mouseDown(e);
    return;
  }
}

/// Called when the slider's value changes, showing or hiding connected sliders based on the current mode.
void TempoSelector::valueChanged() {
  int menu_value = getValue();

  free_slider_->setVisible(menu_value == kSeconds);
  tempo_slider_->setVisible(menu_value != kSeconds && menu_value != kKeytrack);

  if (keytrack_transpose_slider_)
    keytrack_transpose_slider_->setVisible(menu_value == kKeytrack);
  if (keytrack_tune_slider_)
    keytrack_tune_slider_->setVisible(menu_value == kKeytrack);

  SynthSlider::valueChanged();
}

/// Paints an icon representing the current mode (clock, note, triplet notes, keyboard).
/// For dotted tempo, draws an additional dot beside the note.
/// @param g The graphics context.
void TempoSelector::paint(Graphics& g) {
  g.setColour(findColour(Skin::kIconSelectorIcon, true));

  Path path;
  int value = getValue();
  if (value == kSeconds)
    path = Paths::clock();
  else if (value == kTempo || value == kTempoDotted)
    path = Paths::note();
  else if (value == kTempoTriplet)
    path = Paths::tripletNotes();
  else if (value == kKeytrack)
    path = Paths::keyboardBordered();

  g.fillPath(path, path.getTransformToScaleToFit(getLocalBounds().toFloat(), true));
  if (value == kTempoDotted) {
    float dot_width = getWidth() / 8.0f;
    g.fillEllipse(3.0f * getWidth() / 4.0f - dot_width / 2.0f, getHeight() / 2.0f, dot_width, dot_width);
  }
}

/// Sets the slider that will be shown when in Seconds mode.
/// @param slider The slider for free (seconds) mode.
void TempoSelector::setFreeSlider(Slider* slider) {
  bool free_slider = getValue() == kSeconds;

  free_slider_ = slider;
  free_slider_->setVisible(free_slider);
}

/// Sets the slider that will be shown when in a tempo-based mode.
/// @param slider The tempo slider.
void TempoSelector::setTempoSlider(Slider* slider) {
  bool visible = (getValue() != kSeconds) && (getValue() != kKeytrack);

  tempo_slider_ = slider;
  tempo_slider_->setVisible(visible);
}

/// Sets the transpose slider for keytrack mode.
/// @param slider The keytrack transpose slider.
void TempoSelector::setKeytrackTransposeSlider(Slider* slider) {
  bool visible = getValue() == kKeytrack;

  keytrack_transpose_slider_ = slider;
  keytrack_transpose_slider_->setVisible(visible);
}

/// Sets the tune slider for keytrack mode.
/// @param slider The keytrack tune slider.
void TempoSelector::setKeytrackTuneSlider(Slider* slider) {
  bool visible = getValue() == kKeytrack;

  keytrack_tune_slider_ = slider;
  keytrack_tune_slider_->setVisible(visible);
}
