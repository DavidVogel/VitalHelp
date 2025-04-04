#include "text_look_and_feel.h"

#include "skin.h"
#include "fonts.h"
#include "synth_button.h"
#include "synth_section.h"
#include "synth_slider.h"

TextLookAndFeel::TextLookAndFeel() { }

void TextLookAndFeel::drawRotarySlider(Graphics& g, int x, int y, int width, int height,
                                       float slider_t, float start_angle, float end_angle,
                                       Slider& slider) {
  // Draws text in place of a rotary arc, showing value as text
  static constexpr float kTextPercentage = 0.5f;

  SynthSlider* s_slider = dynamic_cast<SynthSlider*>(&slider);
  float text_percentage = kTextPercentage;
  bool active = true;
  String text = slider.getTextFromValue(slider.getValue());
  float offset = 0.0f;
  float font_size = slider.getHeight() * text_percentage;
  if (s_slider) {
    text_percentage = s_slider->getTextHeightPercentage();
    font_size = slider.getHeight() * text_percentage;
    active = s_slider->isActive();
    text = s_slider->getSliderTextFromValue(slider.getValue());

    offset = s_slider->findValue(Skin::kTextComponentOffset);
    if (text_percentage == 0.0f)
      font_size = s_slider->findValue(Skin::kTextComponentFontSize);
  }

  Colour text_color = slider.findColour(Skin::kTextComponentText, true);
  if (!active)
    text_color = text_color.withMultipliedAlpha(0.5f);

  g.setColour(text_color);
  g.setFont(Fonts::instance()->proportional_light().withPointHeight(font_size));
  g.drawText(text, x, y + std::round(offset), width, height, Justification::centred, false);
}

void TextLookAndFeel::drawToggleButton(Graphics& g, ToggleButton& button, bool hover, bool is_down) {
  // Draws a text-based toggle button
  static constexpr float kTextPercentage = 0.7f;
  static constexpr float kTextShrinkage = 0.98f;

  bool toggled = button.getToggleState();
  SynthButton* s_button = dynamic_cast<SynthButton*>(&button);
  const std::string* string_lookup = nullptr;
  if (s_button) {
    string_lookup = s_button->getStringLookup();
  }

  // Color based on state
  if (toggled && string_lookup == nullptr) {
    if (is_down)
      g.setColour(button.findColour(Skin::kIconButtonOnPressed, true));
    else if (hover)
      g.setColour(button.findColour(Skin::kIconButtonOnHover, true));
    else
      g.setColour(button.findColour(Skin::kIconButtonOn, true));
  }
  else {
    if (is_down)
      g.setColour(button.findColour(Skin::kIconButtonOffPressed, true));
    else if (hover)
      g.setColour(button.findColour(Skin::kIconButtonOffHover, true));
    else
      g.setColour(button.findColour(Skin::kIconButtonOff, true));
  }

  String text = button.getButtonText();
  if (string_lookup) {
    int index = toggled ? 1 : 0;
    text = string_lookup[index];
  }

  int rounding = 0;
  float text_percentage = kTextPercentage;
  if (is_down)
    text_percentage *= kTextShrinkage;

  float font_size = button.getHeight() * text_percentage;
  SynthSection* section = button.findParentComponentOfClass<SynthSection>();
  if (section) {
    font_size = section->findValue(Skin::kButtonFontSize);
    rounding = section->findValue(Skin::kWidgetRoundedCorner);
  }

  if (text.isEmpty())
    g.fillRoundedRectangle(button.getLocalBounds().toFloat(), rounding);
  else {
    g.setFont(Fonts::instance()->monospace().withPointHeight(font_size));
    g.drawText(text, 0, 0, button.getWidth(), button.getHeight(), Justification::centred);
  }
}

void TextLookAndFeel::drawTickBox(Graphics& g, Component& component,
                                  float x, float y, float w, float h, bool ticked,
                                  bool enabled, bool mouse_over, bool button_down) {
  // Minimal checkbox drawing: if ticked, draw a small red fill inside
  float border_width = 1.5f;
  if (ticked) {
    g.setColour(Colours::red);
    g.fillRect(x + 3 * border_width, y + 3 * border_width,
               w - 6 * border_width, h - 6 * border_width);
  }
}

void TextLookAndFeel::drawLabel(Graphics& g, Label& label) {
  // Set label text color to body text
  Colour text = label.findColour(Skin::kBodyText, true);
  label.setColour(Label::textColourId, text);

  DefaultLookAndFeel::drawLabel(g, label);
}

void TextLookAndFeel::drawComboBox(Graphics& g, int width, int height, bool is_down,
                                   int button_x, int button_y, int button_w, int button_h, ComboBox& box) {
  // ComboBox with text styling
  Colour background = box.findColour(Skin::kTextComponentBackground, true);
  Colour text = box.findColour(Skin::kBodyText, true);
  Colour caret = box.findColour(Skin::kTextEditorCaret, true);

  box.setColour(ComboBox::backgroundColourId, background);
  box.setColour(ComboBox::arrowColourId, caret);
  box.setColour(ComboBox::outlineColourId, Colours::transparentBlack);
  box.setColour(ComboBox::textColourId, text);

  DefaultLookAndFeel::drawComboBox(g, width, height, is_down, button_x, button_y, button_w, button_h, box);
}
