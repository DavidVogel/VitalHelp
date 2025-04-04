#pragma once

#include "JuceHeader.h"
#include "modulation_button.h"

/**
 * @class ModulationTabSelector
 * @brief A section of the GUI providing multiple modulation sources as tabs.
 *
 * The ModulationTabSelector displays a number of ModulationButton elements, each representing
 * a modulation source. It can show or hide these sources based on how many are currently in use,
 * and allows the user to select one modulation source at a time. Depending on configuration,
 * these sources are displayed either vertically or horizontally, and their visibility adjusts
 * dynamically based on connected modulations.
 */
class ModulationTabSelector : public SynthSection, public ModulationButton::Listener {
public:
    /**
     * @class Listener
     * @brief Interface for objects interested in ModulationTabSelector events.
     */
    class Listener {
    public:
        virtual ~Listener() = default;

        /**
         * @brief Called when a modulation source within this selector is selected.
         * @param selector The ModulationTabSelector that triggered the event.
         * @param index The index of the selected modulation source.
         */
        virtual void modulationSelected(ModulationTabSelector* selector, int index) = 0;
    };

    /**
     * @brief Constructs a ModulationTabSelector with a given prefix and number of tabs.
     * @param prefix A string prefix for naming each modulation source (e.g., "LFO").
     * @param number The number of modulation sources (tabs) to create.
     */
    ModulationTabSelector(std::string prefix, int number);

    /**
     * @brief Constructs a ModulationTabSelector with custom names for each tab.
     * @param name An identifier for this selector.
     * @param number The number of modulation sources.
     * @param names An array of C-style strings naming each modulation source.
     */
    ModulationTabSelector(String name, int number, const char** names);

    /**
     * @brief Destructor.
     */
    virtual ~ModulationTabSelector();

    /**
     * @brief Paints the component background and tab shadows.
     * @param g The graphics context.
     */
    void paintBackground(Graphics& g) override;

    /**
     * @brief Paints a shadow behind the tabs.
     * @param g The graphics context.
     */
    void paintTabShadow(Graphics& g) override;

    /**
     * @brief Called when the component is resized.
     *
     * Updates the layout of displayed modulation tabs.
     */
    void resized() override;

    /**
     * @brief Checks the number of modulations that should be displayed and updates the layout.
     * @param should_repaint If true, triggers a repaint if the count changes.
     */
    void checkNumShown(bool should_repaint);

    /**
     * @brief Resets the tab selector to its default state.
     *
     * Deselects all modulation sources and shows the initial ones if selections are enabled.
     */
    void reset() override;

    /**
     * @brief Called when a modulation button within this selector is clicked.
     * @param source The ModulationButton that was clicked.
     */
    void modulationClicked(ModulationButton* source) override;

    /**
     * @brief Called when a modulation connection changes (e.g., a new connection added or removed).
     */
    void modulationConnectionChanged() override;

    /**
     * @brief Called when modulation mapping ends.
     */
    void endModulationMap() override;

    /**
     * @brief Called when all modulations for a source have been cleared.
     */
    void modulationCleared() override;

    /**
     * @brief Registers a listener to receive events from this ModulationTabSelector.
     * @param listener The listener to add.
     */
    void addListener(Listener* listener) { listeners_.push_back(listener); }

    /**
     * @brief Registers all modulation buttons with a given SynthSection.
     * @param synth_section The SynthSection to register with.
     */
    void registerModulationButtons(SynthSection* synth_section);

    /**
     * @brief Sets the font size used for the modulation button labels.
     * @param font_size The font size in points.
     */
    void setFontSize(float font_size);

    /**
     * @brief Sets whether tabs should be arranged vertically.
     * @param vertical True for vertical layout, false for horizontal.
     */
    void setVertical(bool vertical) { vertical_ = vertical; }

    /**
     * @brief Enables the ability for one of the tabs to remain selected.
     */
    void enableSelections() { selections_enabled_ = true; modulation_buttons_[0]->select(true); }

    /**
     * @brief Sets the minimum number of modulations to show before revealing additional ones.
     * @param num The minimum number of visible modulations.
     */
    void setMinModulationsShown(int num) { min_modulations_shown_ = num; }

    /**
     * @brief Configures whether the tabs should connect visually to the right.
     * @param connect True to connect tabs on their right side.
     */
    void connectRight(bool connect) {
        for (auto& modulation_button : modulation_buttons_)
            modulation_button->setConnectRight(connect);
    }

    /**
     * @brief Gets a pointer to a specific ModulationButton by index.
     * @param index The zero-based index of the modulation button.
     * @return A pointer to the ModulationButton at the given index.
     */
    ModulationButton* getButton(int index) { return modulation_buttons_[index].get(); }

    /**
     * @brief Enables or disables border drawing for each tab.
     * @param draw True to draw borders, false otherwise.
     */
    void drawBorders(bool draw) {
        for (auto& button : modulation_buttons_)
            button->setDrawBorder(draw);
    }

private:
    /**
     * @brief Gets the index of a modulation source based on its name.
     * @param name The name of the modulation source.
     * @return The index of the modulation source, or 0 if not found.
     */
    int getModulationIndex(String name);

    /**
     * @brief Determines how many modulations should currently be visible.
     * @return The number of modulations to show.
     */
    int getNumModulationsToShow();

    std::vector<std::unique_ptr<ModulationButton>> modulation_buttons_; ///< The modulation buttons representing sources.
    std::vector<Listener*> listeners_; ///< Registered listeners for this selector.
    bool vertical_;                    ///< Whether tabs are arranged vertically or horizontally.
    bool selections_enabled_;          ///< Whether one tab can remain selected.
    int min_modulations_shown_;        ///< Minimum number of modulations to show.
    int num_shown_;                    ///< Current number of displayed modulations.

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(ModulationTabSelector)
};
