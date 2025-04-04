#pragma once

#include "JuceHeader.h"
#include "synth_section.h"

class IncrementerButtons;
class TextSelector;
class OscillatorOptions;
class OscillatorSection;
class OscillatorUnison;

/**
 * @class OscillatorAdvancedSection
 * @brief A UI section that provides advanced oscillator controls, including oscillator options and unison settings.
 */
class OscillatorAdvancedSection : public SynthSection {
public:
    /**
     * @brief Constructs an OscillatorAdvancedSection for a given oscillator index.
     * @param index The oscillator index (e.g., 1, 2, 3...).
     * @param mono_modulations A map of monophonic modulations used by the oscillator.
     * @param poly_modulations A map of polyphonic modulations used by the oscillator.
     */
    OscillatorAdvancedSection(int index, const vital::output_map& mono_modulations,
                              const vital::output_map& poly_modulations);

    /**
     * @brief Destructor.
     */
    virtual ~OscillatorAdvancedSection();

    /**
     * @brief Paints the background of this section by painting the children's backgrounds.
     * @param g The graphics context to use for drawing.
     */
    void paintBackground(Graphics& g) override { paintChildrenBackgrounds(g); }

    /**
     * @brief Called when this component is resized. Resets the bounds of child components.
     */
    void resized() override;

    /**
     * @brief Passes a reference to the associated OscillatorSection to allow control synchronization.
     * @param oscillator A pointer to the OscillatorSection instance.
     */
    void passOscillatorSection(const OscillatorSection* oscillator);

private:
    std::unique_ptr<OscillatorOptions> oscillator_options_; ///< The oscillator options subsection.
    std::unique_ptr<OscillatorUnison> oscillator_unison_;   ///< The oscillator unison subsection.

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(OscillatorAdvancedSection)
};
