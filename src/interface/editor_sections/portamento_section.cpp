#include "portamento_section.h"

#include "fonts.h"
#include "synth_strings.h"
#include "synth_button.h"
#include "synth_slider.h"
#include "text_look_and_feel.h"
#include "curve_look_and_feel.h"

PortamentoSection::PortamentoSection(String name) : SynthSection(name) {
  // Create and configure the portamento time slider.
  portamento_ = std::make_unique<SynthSlider>("portamento_time");
  addSlider(portamento_.get());
  portamento_->setSliderStyle(Slider::RotaryHorizontalVerticalDrag);

  // Create and configure the portamento slope slider.
  portamento_slope_ = std::make_unique<SynthSlider>("portamento_slope");
  addSlider(portamento_slope_.get());
  portamento_slope_->setSliderStyle(Slider::RotaryHorizontalVerticalDrag);
  portamento_slope_->setLookAndFeel(CurveLookAndFeel::instance());

  // Button to toggle octave scaling of portamento.
  portamento_scale_ = std::make_unique<SynthButton>("portamento_scale");
  addButton(portamento_scale_.get());
  portamento_scale_->setButtonText("OCTAVE SCALE");
  portamento_scale_->setLookAndFeel(TextLookAndFeel::instance());

  // Button to force glide always on notes.
  portamento_force_ = std::make_unique<SynthButton>("portamento_force");
  addButton(portamento_force_.get());
  portamento_force_->setButtonText("ALWAYS GLIDE");
  portamento_force_->setLookAndFeel(TextLookAndFeel::instance());

  // Button to enable legato mode.
  legato_ = std::make_unique<SynthButton>("legato");
  legato_->setButtonText("LEGATO");
  addButton(legato_.get());
  legato_->setLookAndFeel(TextLookAndFeel::instance());

  setSkinOverride(Skin::kKeyboard);
}

PortamentoSection::~PortamentoSection() { }

void PortamentoSection::paintBackground(Graphics& g) {
  // Paint the main body and border.
  paintBody(g);
  paintBorder(g);

  // Draw the portamento slider shadow.
  portamento_->drawShadow(g);

  // Set label font and draw labels for controls.
  setLabelFont(g);
  drawLabelForComponent(g, TRANS("GLIDE"), portamento_.get());

  Rectangle<int> slope_bounds = portamento_slope_->getBounds().withBottom(getHeight() - getWidgetMargin());
  drawTextComponentBackground(g, slope_bounds, true);
  drawLabel(g, TRANS("SLOPE"), slope_bounds, true);

  paintOpenGlChildrenBackgrounds(g);
}

void PortamentoSection::resized() {
  int height = getHeight();
  int buttons_width = 3 * getWidth() / 8;
  int buttons_x = getWidth() - buttons_width;
  int widget_margin = findValue(Skin::kWidgetMargin);
  int internal_margin = widget_margin / 2;
  float button_height = (height - 2 * (widget_margin + internal_margin)) / 3.0f;

  // Position buttons in a column on the right side.
  portamento_force_->setBounds(buttons_x, widget_margin, buttons_width - widget_margin, button_height);
  legato_->setBounds(buttons_x, height - widget_margin - button_height,
                     buttons_width - widget_margin, button_height);
  portamento_scale_->setBounds(buttons_x, portamento_force_->getBottom() + internal_margin,
                               buttons_width - widget_margin,
                               legato_->getY() - portamento_force_->getBottom() - 2 * internal_margin);

  // Place knobs (sliders) on the left side.
  Rectangle<int> knobs_bounds(0, 0, buttons_x, height);
  placeKnobsInArea(knobs_bounds, { portamento_.get(), portamento_slope_.get() });

  Rectangle<int> slope_bounds = portamento_slope_->getBounds().withTop(getWidgetMargin());

  int bottom = getLabelBackgroundBounds(portamento_slope_->getBounds(), true).getY();
  portamento_slope_->setBounds(slope_bounds.withBottom(bottom));
  SynthSection::resized();
}

void PortamentoSection::sliderValueChanged(Slider* changed_slider) {
  // If the portamento time changes, activate/deactivate slope control depending on the value.
  if (changed_slider == portamento_.get())
    portamento_slope_->setActive(portamento_->getValue() != portamento_->getMinimum());

  SynthSection::sliderValueChanged(changed_slider);
}

void PortamentoSection::setAllValues(vital::control_map& controls) {
  // Update all controls and active states based on incoming values.
  SynthSection::setAllValues(controls);
  portamento_slope_->setActive(portamento_->getValue() != portamento_->getMinimum());
}
