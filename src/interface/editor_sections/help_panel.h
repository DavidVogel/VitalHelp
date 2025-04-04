#pragma once

#include "JuceHeader.h"
#include "synth_section.h"

// model this on header_section.h
// save the listener for after getting the panel to show
class HelpPanel : public SynthSection {
public:


  /**
   * @brief Constructs the HelpPanel.
   */
  HelpPanel();
  ~HelpPanel() override = default;

//  void setHelpText(const String& text);
  /**
   * @brief Paints the background of the help panel.
   * @param g The graphics context.
   */
  void paintBackground(Graphics& g) override;

  /**
   * @brief Resizes and rearranges child components within the help panel.
   */
  void resized() override;

  /**
   * @brief Resets the header, typically after loading a preset or making other state changes.
   */
  void reset() override;


private:
  String help_text_;
};
