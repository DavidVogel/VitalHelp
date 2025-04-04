#include "preset_selector.h"

#include "skin.h"
#include "default_look_and_feel.h"
#include "fonts.h"

/// Constructs a new PresetSelector with default values and initializes child components.
PresetSelector::PresetSelector() : SynthSection("preset_selector"),
                                   font_height_ratio_(kDefaultFontHeightRatio),
                                   round_amount_(0.0f), hover_(false), text_component_(false) {
  static const PathStrokeType arrow_stroke(0.05f, PathStrokeType::JointStyle::curved,
                                           PathStrokeType::EndCapStyle::rounded);

  text_ = std::make_unique<PlainTextComponent>("Text", "Init");
  text_->setFontType(PlainTextComponent::kTitle);
  text_->setInterceptsMouseClicks(false, false);
  addOpenGlComponent(text_.get());
  text_->setScissor(true);
  Path prev_line, prev_shape, next_line, next_shape;

  // Previous preset button
  prev_preset_ = std::make_unique<OpenGlShapeButton>("Prev");
  addAndMakeVisible(prev_preset_.get());
  addOpenGlComponent(prev_preset_->getGlComponent());
  prev_preset_->addListener(this);
  prev_line.startNewSubPath(0.65f, 0.3f);
  prev_line.lineTo(0.35f, 0.5f);
  prev_line.lineTo(0.65f, 0.7f);

  arrow_stroke.createStrokedPath(prev_shape, prev_line);
  prev_shape.addLineSegment(Line<float>(0.0f, 0.0f, 0.0f, 0.0f), 0.2f);
  prev_shape.addLineSegment(Line<float>(1.0f, 1.0f, 1.0f, 1.0f), 0.2f);
  prev_preset_->setShape(prev_shape);

  // Next preset button
  next_preset_ = std::make_unique<OpenGlShapeButton>("Next");
  addAndMakeVisible(next_preset_.get());
  addOpenGlComponent(next_preset_->getGlComponent());
  next_preset_->addListener(this);
  next_line.startNewSubPath(0.35f, 0.3f);
  next_line.lineTo(0.65f, 0.5f);
  next_line.lineTo(0.35f, 0.7f);

  arrow_stroke.createStrokedPath(next_shape, next_line);
  next_shape.addLineSegment(Line<float>(0.0f, 0.0f, 0.0f, 0.0f), 0.2f);
  next_shape.addLineSegment(Line<float>(1.0f, 1.0f, 1.0f, 1.0f), 0.2f);
  next_preset_->setShape(next_shape);
}

/// Destructor.
PresetSelector::~PresetSelector() { }

/// Paints the background of the PresetSelector.
/// Uses skin-specific colors and rounded corners.
void PresetSelector::paintBackground(Graphics& g) {
  float round_amount = findValue(Skin::kWidgetRoundedCorner);
  g.setColour(findColour(Skin::kPopupSelectorBackground, true));
  g.fillRoundedRectangle(0, 0, getWidth(), getHeight(), round_amount);
}

/// Updates the layout of child components when resized.
void PresetSelector::resized() {
  SynthSection::resized();

  if (text_component_) {
    SynthSection* parent = findParentComponentOfClass<SynthSection>();
    int button_height = parent->findValue(Skin::kTextComponentFontSize);
    int offset = parent->findValue(Skin::kTextComponentOffset);
    int button_y = (getHeight() - button_height) / 2 + offset;
    prev_preset_->setBounds(0, button_y, button_height, button_height);
    next_preset_->setBounds(getWidth() - button_height, button_y, button_height, button_height);
    text_->setBounds(getLocalBounds().translated(0, offset));
    text_->setTextSize(button_height);
  }
  else {
    int height = getHeight();
    text_->setBounds(Rectangle<int>(height, 0, getWidth() - 2 * height, height));
    text_->setTextSize(height * font_height_ratio_);
    prev_preset_->setBounds(0, 0, height, height);
    next_preset_->setBounds(getWidth() - height, 0, height, height);
    text_->setColor(findColour(Skin::kPresetText, true));
  }
}

/// Handles mouse-down events on the PresetSelector.
/// Forwards the event to textMouseDown().
/// @param e The mouse event.
void PresetSelector::mouseDown(const MouseEvent& e) {
  textMouseDown(e);
}

/// Handles mouse-up events on the PresetSelector.
/// Forwards the event to textMouseUp().
/// @param e The mouse event.
void PresetSelector::mouseUp(const MouseEvent& e) {
  textMouseUp(e);
}

/// Handles button clicks for the previous/next preset buttons.
/// @param clicked_button The button that was clicked.
void PresetSelector::buttonClicked(Button* clicked_button) {
  if (clicked_button == prev_preset_.get())
    clickPrev();
  else if (clicked_button == next_preset_.get())
    clickNext();
}

/// Sets the displayed text.
/// @param text The new text to display.
void PresetSelector::setText(String text) {
  text_->setText(text);
}

/// Sets the displayed text by concatenating three strings with spacing.
/// @param left The left portion of the text.
/// @param center The center portion of the text.
/// @param right The right portion of the text.
void PresetSelector::setText(String left, String center, String right) {
  text_->setText(left + "  " + center + "  " + right);
}

/// Simulates a click on the "previous" preset button by notifying all listeners.
void PresetSelector::clickPrev() {
  for (Listener* listener : listeners_)
    listener->prevClicked();
}

/// Simulates a click on the "next" preset button by notifying all listeners.
void PresetSelector::clickNext() {
  for (Listener* listener : listeners_)
    listener->nextClicked();
}

/// Handles text mouse-down events by notifying all listeners.
/// @param e The mouse event.
void PresetSelector::textMouseDown(const MouseEvent& e) {
  for (Listener* listener : listeners_)
    listener->textMouseDown(e);
}

/// Handles text mouse-up events by notifying all listeners.
/// @param e The mouse event.
void PresetSelector::textMouseUp(const MouseEvent& e) {
  for (Listener* listener : listeners_)
    listener->textMouseUp(e);
}
