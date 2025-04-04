/// @file tab_selector.cpp
/// @brief Implements the TabSelector class for displaying and selecting tabs.

#include "tab_selector.h"

#include "fonts.h"
#include "skin.h"

/// Constructs a TabSelector with a given name.
/// Sets default look-and-feel values and initializes the image component.
TabSelector::TabSelector(String name) : Slider(name), font_height_percent_(kDefaultFontHeightPercent), active_(true) {
  image_component_.setComponent(this);
  image_component_.setScissor(true);

  setTextBoxStyle(Slider::NoTextBox, true, 0, 0);
  setColour(Slider::backgroundColourId, Colour(0xff303030));
  setColour(Slider::textBoxOutlineColourId, Colour(0x00000000));
  setRange(0, 1, 1);
}

/// Paints the tabs and the highlight for the currently selected tab.
/// Draws a highlight line, tab names, and uses the appropriate colors based on the active state.
void TabSelector::paint(Graphics& g) {
  static constexpr float kLightHeightPercent = 0.08f;
  int selected = getValue();
  int num_types = getMaximum() - getMinimum() + 1;
  int from_highlight = getTabX(selected);
  int to_highlight = getTabX(selected + 1);
  int light_height = std::max<int>(getHeight() * kLightHeightPercent, 1);

  Colour highlight_color = findColour(Skin::kWidgetPrimary1, true);
  if (!active_)
    highlight_color = highlight_color.withSaturation(0.0f);

  g.setColour(findColour(Skin::kLightenScreen, true));
  g.fillRect(0, 0, getWidth(), light_height);

  g.setColour(highlight_color);
  g.fillRect(from_highlight, 0, to_highlight - from_highlight, light_height);

  g.setFont(Fonts::instance()->proportional_light().withPointHeight(getHeight() * font_height_percent_));
  for (int i = 0; i < num_types && i < names_.size(); ++i) {
    std::string name = names_[i];
    int from_x = getTabX(i);
    int to_x = getTabX(i + 1);
    if (i == selected)
      g.setColour(highlight_color);
    else
      g.setColour(findColour(Skin::kTextComponentText, true));

    g.drawText(name, from_x, 0, to_x - from_x, getHeight(), Justification::centred);
  }
}

/// Handles mouse events to select a tab based on the mouse position.
/// @param e The mouse event.
void TabSelector::mouseEvent(const juce::MouseEvent &e) {
  float x = e.getPosition().getX();
  int index = x * (getMaximum() + 1) / getWidth();
  setValue(index);
}

/// Called when the mouse button is pressed on the TabSelector.
/// @param e The mouse event.
void TabSelector::mouseDown(const juce::MouseEvent &e) {
  mouseEvent(e);
}

/// Called when the mouse is dragged while the button is held down on the TabSelector.
/// @param e The mouse event.
void TabSelector::mouseDrag(const juce::MouseEvent &e) {
  mouseEvent(e);
}

/// Called when the mouse button is released on the TabSelector.
/// @param e The mouse event.
void TabSelector::mouseUp(const juce::MouseEvent &e) {
  mouseEvent(e);
}

/// Computes the x-position boundary for a given tab index.
/// Splits the width evenly among the number of tabs.
/// @param position The tab index (0-based).
/// @return The x-position for the start of that tab.
int TabSelector::getTabX(int position) {
  int num_types = getMaximum() - getMinimum() + 1;
  return std::round(float((getWidth() + 1) * position) / num_types);
}

