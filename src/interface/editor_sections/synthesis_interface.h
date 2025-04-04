#pragma once

#include "JuceHeader.h"

#include "filter_section.h"
#include "oscillator_section.h"
#include "sample_section.h"
#include "synth_section.h"

/**
 * @class SynthesisInterface
 * @brief A top-level synthesis section that combines oscillators, samples, and filters.
 *
 * The SynthesisInterface composes multiple OscillatorSection, SampleSection,
 * and FilterSection objects into one cohesive interface. It handles their layout
 * and interconnections, such as routing oscillators and samples into filters.
 */
class SynthesisInterface : public SynthSection,
                           public OscillatorSection::Listener,
                           public SampleSection::Listener,
                           public FilterSection::Listener {
public:
    /**
     * @brief Constructs the SynthesisInterface.
     * @param auth Pointer to Authentication (for online features, if any).
     * @param mono_modulations Mono modulation outputs map.
     * @param poly_modulations Polyphonic modulation outputs map.
     */
    SynthesisInterface(Authentication* auth,
                       const vital::output_map& mono_modulations,
                       const vital::output_map& poly_modulations);

    /** @brief Destructor. */
    virtual ~SynthesisInterface();

    /**
     * @brief Paints the background of the synthesis interface.
     * @param g The Graphics context.
     */
    void paintBackground(Graphics& g) override;

    /**
     * @brief Handles component resizing and lays out child components.
     */
    void resized() override;

    /**
     * @brief Called when the visibility of this interface changes.
     *        Used here to load browser states for oscillators.
     */
    void visibilityChanged() override;

    /**
     * @brief Gives keyboard focus to this interface.
     */
    void setFocus() { grabKeyboardFocus(); }

    /**
     * @brief Called when the distortion type changes in one of the oscillators.
     * @param section The oscillator section that changed.
     * @param type The new distortion type.
     */
    void distortionTypeChanged(OscillatorSection* section, int type) override;

    /**
     * @brief Called when the oscillator routing destination changes.
     * @param section The oscillator section that changed.
     * @param destination The new destination.
     */
    void oscillatorDestinationChanged(OscillatorSection* section, int destination) override;

    /**
     * @brief Called when the sample routing destination changes.
     * @param section The sample section that changed.
     * @param destination The new destination.
     */
    void sampleDestinationChanged(SampleSection* section, int destination) override;

    /**
     * @brief Called when a filter section changes to serial routing.
     * @param section The filter section that changed.
     */
    void filterSerialSelected(FilterSection* section) override;

    /**
     * @brief Called when oscillator input to a filter is toggled.
     * @param section The filter section.
     * @param index The oscillator index.
     * @param on True if input is turned on, false if off.
     */
    void oscInputToggled(FilterSection* section, int index, bool on) override;

    /**
     * @brief Called when sample input to a filter is toggled.
     * @param section The filter section.
     * @param on True if input is turned on, false if off.
     */
    void sampleInputToggled(FilterSection* section, bool on) override;

    /**
     * @brief Gets a pointer to the wave frame slider for a given oscillator.
     * @param index The oscillator index.
     * @return Pointer to the Slider controlling wave frame.
     */
    Slider* getWaveFrameSlider(int index) { return oscillators_[index]->getWaveFrameSlider(); }

    /**
     * @brief Gets the bounds of a given oscillator section.
     * @param index The oscillator index.
     * @return The bounds of the oscillator section.
     */
    Rectangle<int> getOscillatorBounds(int index) { return oscillators_[index]->getBounds(); }

    /**
     * @brief Gets a const pointer to a given oscillator section.
     * @param index The oscillator index.
     * @return Const pointer to the OscillatorSection.
     */
    const OscillatorSection* getOscillatorSection(int index) const { return oscillators_[index].get(); }

    /**
     * @brief Sets the name (e.g. wavetable name) for a given oscillator.
     * @param index The oscillator index.
     * @param name The new name to display.
     */
    void setWavetableName(int index, String name) { oscillators_[index]->setName(name); }

    /**
     * @brief Gets the first filter section.
     * @return Pointer to the first filter section.
     */
    FilterSection* getFilterSection1() { return filter_section_1_.get(); }

    /**
     * @brief Gets the second filter section.
     * @return Pointer to the second filter section.
     */
    FilterSection* getFilterSection2() { return filter_section_2_.get(); }

    /**
     * @brief Gets a non-const pointer to a given oscillator section.
     * @param index The oscillator index.
     * @return Pointer to the OscillatorSection.
     */
    OscillatorSection* getOscillatorSection(int index) { return oscillators_[index].get(); }

private:
    std::unique_ptr<FilterSection> filter_section_1_;
    std::unique_ptr<FilterSection> filter_section_2_;
    std::unique_ptr<OscillatorSection> oscillators_[vital::kNumOscillators];
    std::unique_ptr<SampleSection> sample_section_;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(SynthesisInterface)
};
