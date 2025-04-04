#include "extra_mod_section.h"

#include "line_map_editor.h"
#include "macro_knob_section.h"
#include "skin.h"
#include "fonts.h"
#include "modulation_button.h"
#include "synth_gui_interface.h"

namespace {
  /**
   * @brief An array of strings representing the IDs of the extra modulations handled by this section.
   */
  const char* kModulationStrings[] = {
     "pitch_wheel",
     "mod_wheel"
  };
}

ExtraModSection::ExtraModSection(String name, SynthGuiData* synth_data) : SynthSection(name) {
  // Initialize the other_modulations_ selector with two buttons: pitch wheel and mod wheel.
  other_modulations_ = std::make_unique<ModulationTabSelector>("OTHER", 2, kModulationStrings);
  other_modulations_->getButton(0)->overrideText("PITCH WHL");
  other_modulations_->getButton(1)->overrideText("MOD WHL");
  other_modulations_->drawBorders(true);
  addSubSection(other_modulations_.get());
  other_modulations_->registerModulationButtons(this);
  other_modulations_->setVertical(true);

  // Create the macro knob section for additional macro controls.
  macro_knobs_ = std::make_unique<MacroKnobSection>("MACRO");
  addSubSection(macro_knobs_.get());
}

ExtraModSection::~ExtraModSection() { }

void ExtraModSection::paintBackground(Graphics& g) {
  // Paint the background behind the other_modulations_ subsection.
  g.saveState();
  Rectangle<int> bounds = getLocalArea(other_modulations_.get(), other_modulations_->getLocalBounds());
  g.reduceClipRegion(bounds);
  g.setOrigin(bounds.getTopLeft());
  other_modulations_->paintBackground(g);
  g.restoreState();

  // Paint backgrounds of any children that require it.
  paintChildrenBackgrounds(g);
}

void ExtraModSection::paintBackgroundShadow(Graphics& g) {
  // Paint a shadow effect behind the other_modulations_ tab area.
  paintTabShadow(g, other_modulations_->getBounds());
  SynthSection::paintBackgroundShadow(g);
}

void ExtraModSection::resized() {
  // Paint a shadow effect behind the other_modulations_ tab area.
  int padding = getPadding();
  int knob_section_height = getKnobSectionHeight();
  int widget_margin = getWidgetMargin();

  // The macro knobs may occupy a significant portion of the space.
  int macro_height = 4 * (2 * knob_section_height - widget_margin + padding) - padding;
  int mod_height = getHeight() - macro_height - padding;

  // Set bounds for macro knobs and other modulations sections.
  macro_knobs_->setBounds(0, 0, getWidth(), macro_height);
  other_modulations_->setBounds(0, macro_height + padding, getWidth(), mod_height);

  // Update base class (SynthSection) layout and font size for the mod tabs.
  SynthSection::resized();
  other_modulations_->setFontSize(getModFontSize());
}
