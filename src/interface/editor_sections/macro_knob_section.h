#pragma once

#include "JuceHeader.h"
#include "synth_section.h"
#include "synth_constants.h"

class MacroLabel;
class SingleMacroSection;

/**
 * @class MacroKnobSection
 * @brief A section of the UI dedicated to displaying and editing multiple macro knobs.
 *
 * The MacroKnobSection displays a series of macro knobs, each represented by a SingleMacroSection.
 * Macros are user-configurable controls that can be assigned to various synth parameters. This section
 * arranges them vertically and provides an interface to rename and manipulate each macro.
 *
 * Inherits from SynthSection to ensure consistent styling and integration with the overall UI.
 */
class MacroKnobSection : public SynthSection {
public:
    /**
     * @brief Constructs a new MacroKnobSection.
     *
     * @param name The name of the section.
     */
    MacroKnobSection(String name);

    /**
     * @brief Destructor.
     */
    ~MacroKnobSection();

    /**
     * @brief Paints the background of the macro knob section.
     *
     * @param g The JUCE Graphics context to use for drawing.
     *
     * This method delegates painting to child components to ensure each macro knob and label
     * is drawn correctly.
     */
    void paintBackground(Graphics& g) override;

    /**
     * @brief Called when the component is resized.
     *
     * Determines the layout of the macro knobs, placing them vertically within the section.
     */
    void resized() override;

private:
    /**
     * @brief An array of SingleMacroSection objects, one for each macro knob.
     */
    std::unique_ptr<SingleMacroSection> macros_[vital::kNumMacros];

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(MacroKnobSection)
};
