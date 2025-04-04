#pragma once
#include "help_panel.h"
#include "skin.h"
#include "fonts.h"

HelpPanel::HelpPanel() : SynthSection("help_panel") { }

//void HelpPanel::setHelpText(const String& text) {
//  help_text_ = text;
//  repaint();
//}

void HelpPanel::paintBackground(Graphics& g) {
//  paintBody(g);
  paintContainer(g);
  g.setColour(findColour(Skin::kTextComponentText, true));
  g.setFont(getLabelFont());
  g.drawText(help_text_, getLocalBounds(), Justification::centred, false);
}

void HelpPanel::resized() {
  SynthSection::resized();
  int padding = findValue(Skin::kPadding);
  int text_height = getHeight() * 0.8f;
  int height = getHeight();
  int width = getWidth();
  help_text_ = help_text_.substring(0, 0);
  setBounds(0, 0, getWidth(), text_height);
  SynthSection::resized();
}

void HelpPanel::reset() {
  SynthSection::reset();
  help_text_ = help_text_.substring(0, 0);
}
