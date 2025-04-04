#include "wave_fold_overlay.h"

#include "skin.h"
#include "wave_frame.h"
#include "synth_slider.h"
#include "text_look_and_feel.h"

WaveFoldOverlay::WaveFoldOverlay() : WavetableComponentOverlay("WAVE FOLDER"), wave_fold_modifier_(nullptr) {
  current_frame_ = nullptr;

  // Initialize wave fold amount slider
  wave_fold_amount_ = std::make_unique<SynthSlider>("wave_fold_amount");
  addSlider(wave_fold_amount_.get());
  wave_fold_amount_->getImageComponent()->setAlwaysOnTop(true);
  wave_fold_amount_->setAlwaysOnTop(true);
  wave_fold_amount_->addListener(this);
  wave_fold_amount_->setRange(1.0f, 32.0f);
  wave_fold_amount_->setDoubleClickReturnValue(true, 1.0f);
  wave_fold_amount_->setLookAndFeel(TextLookAndFeel::instance());
  wave_fold_amount_->setSliderStyle(Slider::RotaryHorizontalVerticalDrag);

  // Set control background titles
  controls_background_.clearTitles();
  controls_background_.addTitle("MULTIPLY");
}

void WaveFoldOverlay::frameSelected(WavetableKeyframe* keyframe) {
  if (keyframe == nullptr) {
    current_frame_ = nullptr;
  }

  // Frame belongs to the wave fold modifier, update UI values
  else if (keyframe->owner() == wave_fold_modifier_) {
    current_frame_ = wave_fold_modifier_->getKeyframe(keyframe->index());
    wave_fold_amount_->setValue(current_frame_->getWaveFoldBoost(), dontSendNotification);
    wave_fold_amount_->redoImage();
  }
}

void WaveFoldOverlay::setEditBounds(Rectangle<int> bounds) {
  static constexpr float kFoldWidthHeightRatio = 4.0f;

  float width = bounds.getHeight() * kFoldWidthHeightRatio;
  setControlsWidth(width);
  int x = bounds.getX() + (bounds.getWidth() - width) / 2;

  int title_height = bounds.getHeight() * WavetableComponentOverlay::kTitleHeightRatio;
  Rectangle<int> slider_bounds(x, bounds.getY() + title_height, width, bounds.getHeight() - title_height);
  WavetableComponentOverlay::setEditBounds(bounds);
  wave_fold_amount_->setBounds(slider_bounds);

  controls_background_.setPositions();
  wave_fold_amount_->redoImage();
}

void WaveFoldOverlay::sliderValueChanged(Slider* moved_slider) {
  if (current_frame_) {
    // Update the current keyframe with the new fold amount
    current_frame_->setWaveFoldBoost(wave_fold_amount_->getValue());
    notifyChanged(false);
  }
}

void WaveFoldOverlay::sliderDragEnded(Slider* moved_slider) {
  // User finished interacting with the slider, commit changes
  notifyChanged(true);
}
