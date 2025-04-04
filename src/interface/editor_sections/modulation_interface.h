#pragma once

#include "JuceHeader.h"
#include "modulation_tab_selector.h"
#include "synth_constants.h"
#include "synth_section.h"

class EnvelopeSection;
class RandomSection;
class LineGenerator;
class LfoSection;
struct SynthGuiData;

/**
 * @class ModulationInterface
 * @brief A user interface section for managing and viewing various modulation sources like envelopes, LFOs, and random generators.
 *
 * The ModulationInterface coordinates multiple modulation-related sections:
 * - EnvelopeSections for envelope generators.
 * - LfoSections for low-frequency oscillators.
 * - RandomSections for random modulation sources.
 *
 * It also includes keyboard-based modulation controls and handles showing and hiding
 * these sections based on user selection through ModulationTabSelectors. Minimum numbers
 * of modulations are shown initially, and users can select specific modulation sources
 * via the tab selectors.
 */
class ModulationInterface  : public SynthSection, public ModulationTabSelector::Listener {
public:
    /**
     * @brief The minimum number of envelopes, LFOs, and random modulations to show by default.
     */
    static constexpr int kMinEnvelopeModulationsToShow = 3; ///< Minimum envelopes to show
    static constexpr int kMinLfoModulationsToShow = 4;      ///< Minimum LFOs to show
    static constexpr int kMinRandomModulationsToShow = 2;   ///< Minimum random modulations to show
    static constexpr int kMinTotalModulations =
            kMinEnvelopeModulationsToShow + kMinLfoModulationsToShow + kMinRandomModulationsToShow;

    /**
     * @brief Constructs the ModulationInterface.
     *
     * @param synth_data A pointer to SynthGuiData containing references to synth modulation maps and data.
     */
    ModulationInterface(SynthGuiData* synth_data);

    /**
     * @brief Destructor.
     */
    ~ModulationInterface();

    /**
     * @brief Paints the background of the modulation interface.
     *
     * @param g The JUCE Graphics context.
     */
    void paintBackground(Graphics& g) override;

    /**
     * @brief Paints shadows behind the modulation sections.
     *
     * @param g The JUCE Graphics context.
     */
    void paintBackgroundShadow(Graphics& g) override;

    /**
     * @brief Called when the component is resized.
     *
     * Arranges the layout of envelopes, LFOs, random sections, and keyboard modulations,
     * as well as their corresponding tab selectors.
     */
    void resized() override;

    /**
     * @brief Resets all visible modulation sections to their default states.
     */
    void reset() override;

    /**
     * @brief Checks and adjusts the number of shown modulations in each category to
     *        maintain consistency with the minimum visible settings.
     */
    void checkNumShown();

    /**
     * @brief Called when a modulation tab is selected.
     *
     * Shows the selected modulation section and hides others in the same category.
     *
     * @param selector The ModulationTabSelector that triggered the event.
     * @param index The selected modulation index.
     */
    void modulationSelected(ModulationTabSelector* selector, int index) override;

    /**
     * @brief Gives keyboard focus to this component.
     */
    void setFocus() { grabKeyboardFocus(); }

private:
    /**
     * @brief An array of EnvelopeSection objects for envelope modulations.
     */
    std::unique_ptr<EnvelopeSection> envelopes_[vital::kNumEnvelopes];

    /**
     * @brief A tab selector for switching between envelope modulations.
     */
    std::unique_ptr<ModulationTabSelector> envelope_tab_selector_;

    /**
     * @brief An array of LfoSection objects for LFO modulations.
     */
    std::unique_ptr<LfoSection> lfos_[vital::kNumLfos];

    /**
     * @brief A tab selector for switching between LFO modulations.
     */
    std::unique_ptr<ModulationTabSelector> lfo_tab_selector_;

    /**
     * @brief An array of RandomSection objects for random modulations.
     */
    std::unique_ptr<RandomSection> random_lfos_[vital::kNumRandomLfos];

    /**
     * @brief A tab selector for switching between random modulation sources.
     */
    std::unique_ptr<ModulationTabSelector> random_tab_selector_;

    /**
     * @brief A tab selector for keyboard-related modulations (top row).
     */
    std::unique_ptr<ModulationTabSelector> keyboard_modulations_top_;

    /**
     * @brief A tab selector for keyboard-related modulations (bottom row).
     */
    std::unique_ptr<ModulationTabSelector> keyboard_modulations_bottom_;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(ModulationInterface)
};
