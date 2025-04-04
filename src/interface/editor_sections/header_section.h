#pragma once

#include "JuceHeader.h"
#include "synth_section.h"
#include "bank_exporter.h"
#include "synth_preset_selector.h"

class BankExporter;
class LogoButton;
class TabSelector;
class Oscilloscope;
class Spectrogram;
class PresetBrowser;
class VolumeSection;

/**
 * @class LogoSection
 * @brief A section at the top of the interface displaying the Vital logo.
 *
 * The LogoSection holds a clickable logo button. When clicked, it can notify listeners to show the "About" section or other relevant information.
 */
class LogoSection : public SynthSection {
  public:
    /**
     * @brief The vertical padding applied to the logo within its section.
     */
    static constexpr float kLogoPaddingY = 2.0f;

    /**
     * @class Listener
     * @brief Interface for objects that need to respond to logo interactions.
     */
    class Listener {
      public:
        virtual ~Listener() { }

        /**
         * @brief Called when the logo is clicked and the "About" section should be shown.
         */
        virtual void showAboutSection() = 0;
    };

    /**
     * @brief Constructs the LogoSection.
     */
    LogoSection();

    /**
     * @brief Positions the logo button within the section.
     */
    void resized() override;

    /**
     * @brief Paints the background of the logo section.
     * @param g The graphics context used for drawing.
     */
    void paintBackground(Graphics& g) override;

    /**
     * @brief Handles button click events. In this case, clicking the logo button.
     * @param clicked_button The button that was clicked.
     */
    void buttonClicked(Button* clicked_button) override;

    /**
     * @brief Registers a listener for the logo interactions.
     * @param listener A pointer to an object implementing the Listener interface.
     */
    void addListener(Listener* listener) { listeners_.push_back(listener); }

  private:
    std::vector<Listener*> listeners_;
    std::unique_ptr<LogoButton> logo_button_;
};

/**
 * @class HeaderSection
 * @brief The topmost section of the synthesizer GUI, displaying the logo, tab selector, preset controls, oscilloscope, and more.
 *
 * The HeaderSection manages:
 * - A logo section (LogoSection)
 * - A tab selector for switching between main sections (e.g., VOICE, EFFECTS, MATRIX, ADVANCED)
 * - A preset selector for loading and saving patches
 * - Volume and display elements like oscilloscope or spectrogram
 * - Temporary tabs and overlays (e.g., preset browser, bank exporter)
 *
 * Listeners can respond to changes such as tab selections, preset loading, or bank exports.
 */
class HeaderSection : public SynthSection, public SaveSection::Listener,
                      public SynthPresetSelector::Listener, public LogoSection::Listener {
  public:
    /**
     * @class Listener
     * @brief Interface for objects that need to be notified of events from the HeaderSection.
     */
    class Listener {
      public:
        virtual ~Listener() { }

        /**
         * @brief Called when the "About" section should be shown.
         */
        virtual void showAboutSection() = 0;

        /**
         * @brief Called when a delete request is made for a given preset file.
         * @param preset The preset File requested for deletion.
         */
        virtual void deleteRequested(File preset) = 0;

        /**
         * @brief Called when a tab is selected by the user.
         * @param index The index of the selected tab.
         */
        virtual void tabSelected(int index) = 0;

        /**
         * @brief Called when a temporary tab (e.g., preset browser) should be cleared and return to the previous tab.
         * @param current_tab The index of the currently active main tab.
         */
        virtual void clearTemporaryTab(int current_tab) = 0;

        /**
         * @brief Called when the preset browser visibility should change.
         * @param visible True if the browser should be visible, false otherwise.
         * @param current_tab The currently selected main tab.
         */
        virtual void setPresetBrowserVisibility(bool visible, int current_tab) = 0;

        /**
         * @brief Called when the bank exporter visibility should change.
         * @param visible True if the bank exporter should be visible, false otherwise.
         * @param current_tab The currently selected main tab.
         */
        virtual void setBankExporterVisibility(bool visible, int current_tab) = 0;

        /**
         * @brief Called after a bank of presets has been imported.
         */
        virtual void bankImported() = 0;
    };

    /**
     * @brief Constructs the HeaderSection.
     */
    HeaderSection();

    /**
     * @brief Paints the background of the header, including tab areas, preset areas, and optional expiration countdown.
     * @param g The graphics context.
     */
    void paintBackground(Graphics& g) override;

    /**
     * @brief Resizes and rearranges child components within the header.
     */
    void resized() override;

    /**
     * @brief Resets the header, typically after loading a preset or making other state changes.
     */
    void reset() override;

    /**
     * @brief Sets parameter values for the header section.
     * @param controls A map of parameter names and values.
     */
    void setAllValues(vital::control_map& controls) override;

    /**
     * @brief Handles button clicks within the header (e.g., toggling spectrogram, exiting temporary tabs).
     * @param clicked_button The button that was clicked.
     */
    void buttonClicked(Button* clicked_button) override;

    /**
     * @brief Handles slider value changes (e.g., when the tab selector moves to a different tab).
     * @param slider The slider that changed value.
     */
    void sliderValueChanged(Slider* slider) override;

    /**
     * @brief Called to show or hide the preset browser.
     * @param visible True to show, false to hide.
     */
    void setPresetBrowserVisibility(bool visible) override;

    /**
     * @brief Called to show or hide the bank exporter.
     * @param visible True to show, false to hide.
     */
    void setBankExporterVisibility(bool visible) override;

    /**
     * @brief Handles the request to delete a given preset.
     * @param preset The preset file to delete.
     */
    void deleteRequested(File preset) override;

    /**
     * @brief Called when a bank of presets has been imported.
     */
    void bankImported() override;

    /**
     * @brief Called after saving a preset, allowing the header to update its state (e.g., mark as unmodified).
     * @param preset The file that was just saved.
     */
    void save(File preset) override;

    /**
     * @brief Sets a temporary tab name, typically when overlaying the preset browser or bank exporter.
     * @param name The name of the temporary tab.
     */
    void setTemporaryTab(String name);

    /**
     * @brief Called when the logo's listener requests the About section.
     */
    void showAboutSection() override {
      for (Listener* listener : listeners_)
        listener->showAboutSection();
    }

    /**
     * @brief Sets the memory used by the oscilloscope for visualization.
     * @param memory Pointer to poly_float memory used by the oscilloscope.
     */
    void setOscilloscopeMemory(const vital::poly_float* memory);

    /**
     * @brief Sets the memory used by the spectrogram for visualization.
     * @param memory Pointer to StereoMemory used by the spectrogram.
     */
    void setAudioMemory(const vital::StereoMemory* memory);

    /**
     * @brief Marks the interface as having unsaved changes.
     */
    void notifyChange();

    /**
     * @brief Marks the interface as having no unsaved changes.
     */
    void notifyFresh();

    /**
     * @brief Assigns the SaveSection used by the header's preset selector.
     * @param save_section A pointer to the SaveSection.
     */
    void setSaveSection(SaveSection* save_section) {
      synth_preset_selector_->setSaveSection(save_section);
      save_section->addSaveListener(this);
    }

    /**
     * @brief Assigns the preset browser to the preset selector.
     * @param browser A pointer to the PresetBrowser.
     */
    void setBrowser(PresetBrowser* browser) { synth_preset_selector_->setBrowser(browser); }

    /**
     * @brief Assigns the bank exporter to the preset selector.
     * @param exporter A pointer to the BankExporter.
     */
    void setBankExporter(BankExporter* exporter) { synth_preset_selector_->setBankExporter(exporter); }

    /**
     * @brief Adds a Listener to receive header events.
     * @param listener A pointer to an object implementing HeaderSection::Listener.
     */
    void addListener(Listener* listener) { listeners_.push_back(listener); }

    /**
     * @brief Sets the horizontal offset for the tab selector, allowing flexible layout adjustments.
     * @param offset The new offset in pixels.
     */
    void setTabOffset(int offset) { tab_offset_ = offset; repaint(); }

  private:
    std::vector<Listener*> listeners_;

    std::unique_ptr<LogoSection> logo_section_;
    std::unique_ptr<TabSelector> tab_selector_;
    int tab_offset_;
    std::unique_ptr<PlainTextComponent> temporary_tab_;
    std::unique_ptr<OpenGlShapeButton> exit_temporary_button_;

    std::unique_ptr<SynthButton> view_spectrogram_;
    std::unique_ptr<Oscilloscope> oscilloscope_;
    std::unique_ptr<Spectrogram> spectrogram_;
    std::unique_ptr<SynthPresetSelector> synth_preset_selector_;
    std::unique_ptr<VolumeSection> volume_section_;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(HeaderSection)
};

