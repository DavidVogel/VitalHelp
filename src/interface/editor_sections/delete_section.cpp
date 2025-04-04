/// @file delete_section.cpp
/// @brief Implements the DeleteSection class for confirming and performing file deletions.

#include "delete_section.h"

#include "skin.h"
#include "fonts.h"

/// Constructs a DeleteSection overlay with a message, filename, and Delete/Cancel buttons.
DeleteSection::DeleteSection(const String& name) : Overlay(name), body_(Shaders::kRoundedRectangleFragment) {
  addOpenGlComponent(&body_);

  delete_button_ = std::make_unique<OpenGlToggleButton>(TRANS("Delete"));
  delete_button_->setText("Delete");
  delete_button_->setUiButton(true);
  delete_button_->addListener(this);
  addAndMakeVisible(delete_button_.get());
  addOpenGlComponent(delete_button_->getGlComponent());

  cancel_button_ = std::make_unique<OpenGlToggleButton>(TRANS("Cancel"));
  cancel_button_->setText("Cancel");
  cancel_button_->setUiButton(false);
  cancel_button_->addListener(this);
  addAndMakeVisible(cancel_button_.get());
  addOpenGlComponent(cancel_button_->getGlComponent());

  delete_text_ = std::make_unique<PlainTextComponent>("Delete", "Are you sure you want to delete this preset?");
  addOpenGlComponent(delete_text_.get());
  delete_text_->setFontType(PlainTextComponent::kLight);
  delete_text_->setTextSize(kTextHeight);
  delete_text_->setJustification(Justification::centred);

  preset_text_ = std::make_unique<PlainTextComponent>("Preset", "");
  addOpenGlComponent(preset_text_.get());
  preset_text_->setFontType(PlainTextComponent::kLight);
  preset_text_->setTextSize(kTextHeight);
  preset_text_->setJustification(Justification::centred);
}

/// Positions the text and buttons inside the confirmation box.
void DeleteSection::resized() {
  body_.setRounding(findValue(Skin::kBodyRounding));
  body_.setColor(findColour(Skin::kBody, true));

  Colour body_text = findColour(Skin::kBodyText, true);
  delete_text_->setColor(body_text);
  preset_text_->setColor(body_text);

  Rectangle<int> delete_rect = getDeleteRect();
  body_.setBounds(delete_rect);

  int padding_x = size_ratio_ * kPaddingX;
  int padding_y = size_ratio_ * kPaddingY;
  int button_height = size_ratio_ * kButtonHeight;

  float button_width = (delete_rect.getWidth() - 3 * padding_x) / 2.0f;
  cancel_button_->setBounds(delete_rect.getX() + padding_x,
                            delete_rect.getBottom() - padding_y - button_height,
                            button_width, button_height);
  delete_button_->setBounds(delete_rect.getX() + button_width + 2 * padding_x,
                            delete_rect.getBottom() - padding_y - button_height,
                            button_width, button_height);

  float text_size = kTextHeight * size_ratio_;
  delete_text_->setTextSize(text_size);
  preset_text_->setTextSize(text_size);

  float text_height = 22.0f * size_ratio_;
  delete_text_->setBounds(delete_rect.getX() + padding_x, delete_rect.getY() + padding_y,
                          delete_rect.getWidth() - 2 * padding_x, text_height);
  preset_text_->setBounds(delete_rect.getX() + padding_x, delete_rect.getY() + padding_y + 30 * size_ratio_,
                          delete_rect.getWidth() - 2 * padding_x, text_height);
  preset_text_->setText(file_.getFileNameWithoutExtension());

  Overlay::resized();
}

/// Sets the visibility of the DeleteSection. Repaints background if becoming visible.
void DeleteSection::setVisible(bool should_be_visible) {
  Overlay::setVisible(should_be_visible);

  if (should_be_visible) {
    Image image(Image::ARGB, 1, 1, false);
    Graphics g(image);
    paintOpenGlChildrenBackgrounds(g);
  }
}

/// Handles mouse up events. If clicked outside the confirmation box, hides the overlay.
/// @param e The mouse event.
void DeleteSection::mouseUp(const MouseEvent &e) {
  if (!getDeleteRect().contains(e.getPosition()))
    setVisible(false);
}

/// Handles button clicks for "Delete" and "Cancel".
/// @param clicked_button The button that was clicked.
void DeleteSection::buttonClicked(Button* clicked_button) {
  if (clicked_button == delete_button_.get()) {
    file_.deleteRecursively();
    setVisible(false);
    for (Listener* listener : listeners_)
      listener->fileDeleted(file_);
  }
  else if (clicked_button == cancel_button_.get())
    setVisible(false);
}

/// Gets the rectangle representing the delete confirmation box.
/// @return The bounding rectangle.
Rectangle<int> DeleteSection::getDeleteRect() {
  int width = kDeleteWidth * size_ratio_;
  int height = kDeleteHeight * size_ratio_;
  int x = (getWidth() - width) / 2;
  int y = (getHeight() - height) / 2;
  return Rectangle<int>(x, y, width, height);
}
