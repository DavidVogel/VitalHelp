#pragma once

#include "JuceHeader.h"
#include "synth_section.h"
#include "modulation_tab_selector.h"

class LineMapEditor;
class MacroKnobSection;
struct SynthGuiData;

/**
 * @class ExtraModSection
 * @brief A section that displays additional modulation controls for the synthesizer, including macro knobs and other modulations.
 *
 * This section provides a user interface area for selecting and controlling extra modulations that are not part of the main modulation interfaces.
 * It includes a set of macro knobs and a vertical tab selector for other modulation sources (e.g., pitch wheel and mod wheel).
 */
class ExtraModSection : public SynthSection {
public:
    /**
     * @brief Constructs the ExtraModSection.
     * @param name The name of this component.
     * @param synth_data A pointer to the SynthGuiData that provides data and state for the GUI.
     */
    ExtraModSection(String name, SynthGuiData* synth_data);

    /**
     * @brief Destructor.
     */
    virtual ~ExtraModSection();

    /**
     * @brief Paints the background of the component.
     *
     * This includes painting the background of the "other modulations" subsection, if necessary.
     * @param g The graphics context to use for drawing.
     */
    void paintBackground(Graphics& g) override;

    /**
     * @brief Paints a background shadow for the component.
     *
     * This method can be used to add shadows or depth effects behind child components.
     * @param g The graphics context to use for drawing.
     */
    void paintBackgroundShadow(Graphics& g) override;

    /**
     * @brief Called when the component is resized.
     *
     * This method updates the layout and positions of the macro knobs and other modulations sections.
     */
    void resized() override;

private:
    /**
     * @brief A tab selector component for additional modulation sources such as pitch and mod wheels.
     */
    std::unique_ptr<ModulationTabSelector> other_modulations_;

    /**
     * @brief A section containing macro knobs that can be assigned to various parameters for quick access.
     */
    std::unique_ptr<MacroKnobSection> macro_knobs_;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(ExtraModSection)
};
