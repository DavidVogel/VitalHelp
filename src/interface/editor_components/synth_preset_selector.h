/// @file synth_preset_selector.h
/// @brief Declares the SynthPresetSelector class, which provides UI elements for selecting, browsing, and managing presets.

#pragma once

#include "JuceHeader.h"
#include "bank_exporter.h"
#include "preset_selector.h"
#include "preset_browser.h"
#include "synth_button.h"
#include "synth_section.h"

/// @class SynthPresetSelector
/// @brief A UI section that allows the user to select, load, save, and browse presets, and manage associated resources.
///
/// The SynthPresetSelector integrates a PresetSelector with additional UI controls for:
/// - Initializing presets
/// - Saving and exporting presets
/// - Importing/Exporting banks
/// - Browsing presets through a PresetBrowser
/// - Managing tunings and skins
class SynthPresetSelector : public SynthSection,
                            public PresetBrowser::Listener,
                            public BankExporter::Listener,
                            public PresetSelector::Listener {
public:
    /// @class Listener
    /// @brief Interface for components that need to respond to preset selector events.
    class Listener {
    public:
        /// Virtual destructor for proper cleanup.
        virtual ~Listener() { }

        /// Called when the preset browser should change visibility.
        /// @param visible True to make visible, false otherwise.
        virtual void setPresetBrowserVisibility(bool visible) = 0;

        /// Called when the bank exporter should change visibility.
        /// @param visible True to make visible, false otherwise.
        virtual void setBankExporterVisibility(bool visible) = 0;

        /// Called when a preset file delete is requested.
        /// @param file The preset file to delete.
        virtual void deleteRequested(File file) = 0;

        /// Called when a bank is successfully imported.
        virtual void bankImported() = 0;
    };

    /// Menu item IDs for popup menus.
    enum MenuItems {
        kCancelled,
        kInitPreset,
        kSavePreset,
        kImportPreset,
        kExportPreset,
        kImportBank,
        kExportBank,
        kBrowsePresets,
        kLoadTuning,
        kClearTuning,
        kOpenSkinDesigner,
        kLoadSkin,
        kClearSkin,
        kLogOut,
        kLogIn,
        kNumMenuItems
    };

    /// Constructor.
    SynthPresetSelector();
    /// Destructor.
    ~SynthPresetSelector();

    /// Resizes and lays out UI components.
    void resized() override;

    /// Paints the background of the selector area (delegated to the PresetSelector).
    /// @param g The graphics context to use.
    void paintBackground(Graphics& g) override { selector_->paintBackground(g); }

    /// Called when a button in this section is clicked.
    /// @param buttonThatWasClicked The button that triggered the event.
    void buttonClicked(Button* buttonThatWasClicked) override;

    /// Called when a new preset is selected in the PresetBrowser.
    /// @param preset The selected preset file.
    void newPresetSelected(File preset) override;

    /// Called when the delete action is requested on a preset in the PresetBrowser.
    /// @param preset The preset file to delete.
    void deleteRequested(File preset) override;

    /// Called when the PresetBrowser is requested to be hidden.
    void hidePresetBrowser() override;

    /// Called when the BankExporter is requested to be hidden.
    void hideBankExporter() override;

    /// Resets the text displayed in the PresetSelector to reflect current state.
    void resetText();

    /// Called when the previous preset button is clicked in the PresetSelector.
    void prevClicked() override;

    /// Called when the next preset button is clicked in the PresetSelector.
    void nextClicked() override;

    /// Called when the text area in the PresetSelector receives a mouse-up event.
    /// @param e The mouse event data.
    void textMouseUp(const MouseEvent& e) override;

    /// Shows a popup menu for preset management.
    /// @param anchor The component used as the position anchor for the popup.
    void showPopupMenu(Component* anchor);

    /// Shows an alternate popup menu (used for advanced/hidden options).
    /// @param anchor The component used as the position anchor for the popup.
    void showAlternatePopupMenu(Component* anchor);

    /// Sets whether the current preset is considered modified.
    /// @param modified True if modified, false otherwise.
    void setModified(bool modified);

    /// Assigns a SaveSection to handle saving presets.
    /// @param save_section The SaveSection to use.
    void setSaveSection(SaveSection* save_section) { save_section_ = save_section; }

    /// Sets the PresetBrowser to use.
    /// @param browser The PresetBrowser instance.
    void setBrowser(PresetBrowser* browser) {
        if (browser_ != browser) {
            browser_ = browser;
            browser_->addListener(this);
        }
    }

    /// Sets the BankExporter to use.
    /// @param bank_exporter The BankExporter instance.
    void setBankExporter(BankExporter* bank_exporter) {
        if (bank_exporter_ != bank_exporter) {
            bank_exporter_ = bank_exporter;
            bank_exporter_->addListener(this);
        }
    }

    /// Shows or hides the preset browser.
    /// @param visible True to show, false to hide.
    void setPresetBrowserVisibile(bool visible);

    /// Makes the bank exporter visible.
    void makeBankExporterVisibile();

    /// Initializes the current preset to a default state.
    void initPreset();

    /// Saves the current preset.
    void savePreset();

    /// Imports a preset from an external file.
    void importPreset();

    /// Exports the current preset to a file.
    void exportPreset();

    /// Imports a bank from an external file.
    void importBank();

    /// Exports the current bank.
    void exportBank();

    /// Loads a tuning file and applies it to the synthesizer.
    void loadTuningFile();

    /// Clears the currently loaded tuning, reverting to default.
    void clearTuning();

    /// Retrieves the current tuning name.
    /// @return A string containing the tuning name.
    std::string getTuningName();

    /// Checks if the current tuning is the default tuning.
    /// @return True if default, false otherwise.
    bool hasDefaultTuning();

    /// Gets the name of the logged-in user.
    /// @return The logged-in user's name or an empty string if not logged in.
    std::string loggedInName();

    /// Signs out the currently logged-in user.
    void signOut();

    /// Opens the sign-in dialog for the user.
    void signIn();

    /// Opens the skin designer tool.
    void openSkinDesigner();

    /// Loads a skin from a file.
    void loadSkin();

    /// Clears the current skin and reverts to the default.
    void clearSkin();

    /// Repaints the interface with the current skin settings.
    void repaintWithSkin();

    /// Shows the preset browser UI.
    void browsePresets();

    /// Adds a listener for events from this preset selector.
    /// @param listener The listener to add.
    void addListener(Listener* listener) { listeners_.push_back(listener); }

private:
    /// Loads a preset from a file directly (used internally).
    /// @param preset The preset file to load.
    void loadFromFile(File& preset);

    std::vector<Listener*> listeners_;             ///< Registered listeners for event notifications.
    std::unique_ptr<Skin> full_skin_;              ///< A full skin instance for customizing the UI.
    Component::SafePointer<Component> skin_designer_; ///< A pointer to an external SkinDesigner component.

    std::unique_ptr<PresetSelector> selector_;     ///< The core preset selector UI element.
    std::unique_ptr<OpenGlShapeButton> menu_button_; ///< Button to show preset management menus.
    std::unique_ptr<OpenGlShapeButton> save_button_; ///< Button to save the current preset.
    BankExporter* bank_exporter_;                  ///< External bank exporter interface.
    PresetBrowser* browser_;                       ///< External preset browser interface.
    SaveSection* save_section_;                    ///< SaveSection for handling preset saving.
    bool modified_;                                ///< Indicates if the current preset is modified.

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(SynthPresetSelector)
};
