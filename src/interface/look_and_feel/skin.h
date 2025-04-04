#pragma once

#include "JuceHeader.h"

#include "json/json.h"

using json = nlohmann::json;

class FullInterface;
class SynthSection;

/**
 * @class Skin
 * @brief Manages the overall color and value theme (or "skin") of the user interface.
 *
 * The Skin class stores a variety of colors and values that determine the appearance and layout
 * of UI components. It supports section-based overrides, allowing different interface sections
 * (e.g., Oscillator, Filter, Envelope) to have unique colors or values if desired.
 *
 * The skin information can be serialized to and from JSON, allowing customization and saving of
 * user-defined skins. It also integrates with JUCE's LookAndFeel system, applying colors and values
 * to components and ensuring consistent UI styling.
 */
class Skin {
  public:
    /**
     * @enum SectionOverride
     * @brief Identifiers for different UI sections that can have color or value overrides.
     */
    enum SectionOverride {
      kNone,
      kLogo,
      kHeader,
      kOverlay,
      kOscillator,
      kSample,
      kSub,
      kFilter,
      kEnvelope,
      kLfo,
      kRandomLfo,
      kVoice,
      kMacro,
      kKeyboard,
      kAllEffects,
      kChorus,
      kCompressor,
      kDelay,
      kDistortion,
      kEqualizer,
      kFxFilter,
      kFlanger,
      kPhaser,
      kReverb,
      kModulationDragDrop,
      kModulationMatrix,
      kPresetBrowser,
      kPopupBrowser,
      kAdvanced,
      kWavetableEditor,
      kNumSectionOverrides
    };

    /**
     * @enum ValueId
     * @brief Identifiers for various UI scaling/spacing values and configuration constants.
     *
     * These values control dimensions, rounding, padding, text sizes, knob sizes, etc.
     */
    enum ValueId {
      kBodyRounding,
      kLabelHeight,
      kLabelBackgroundHeight,
      kLabelBackgroundRounding,
      kLabelOffset,
      kTextComponentLabelOffset,
      kRotaryOptionXOffset,
      kRotaryOptionYOffset,
      kRotaryOptionWidth,
      kTitleWidth,
      kPadding,
      kLargePadding,
      kSliderWidth,
      kTextComponentHeight,
      kTextComponentOffset,
      kTextComponentFontSize,
      kTextButtonHeight,
      kButtonFontSize,
      kKnobArcSize,
      kKnobArcThickness,
      kKnobBodySize,
      kKnobHandleLength,
      kKnobModAmountArcSize,
      kKnobModAmountArcThickness,
      kKnobModMeterArcSize,
      kKnobModMeterArcThickness,
      kKnobOffset,
      kKnobSectionHeight,
      kKnobShadowWidth,
      kKnobShadowOffset,
      kModulationButtonWidth,
      kModulationFontSize,
      kWidgetMargin,
      kWidgetRoundedCorner,
      kWidgetLineWidth,
      kWidgetLineBoost,
      kWidgetFillCenter,
      kWidgetFillFade,
      kWidgetFillBoost,
      kWavetableHorizontalAngle,
      kWavetableVerticalAngle,
      kWavetableDrawWidth,
      kWavetableWaveHeight,
      kWavetableYOffset,
      kNumSkinValueIds,
      kFrequencyDisplay = kNumSkinValueIds,
      kNumAllValueIds,
    };

    /**
     * @enum ColorId
     * @brief Identifiers for all colors used in the UI.
     *
     * Each color maps to a JUCE ColourId on components or is used directly when drawing.
     */
    enum ColorId {
      kInitialColor = 0x42345678,
      kBackground = kInitialColor,
      kBody,
      kBodyHeading,
      kHeadingText,
      kPresetText,
      kBodyText,
      kBorder,
      kLabelBackground,
      kLabelConnection,
      kPowerButtonOn,
      kPowerButtonOff,

      kOverlayScreen,
      kLightenScreen,
      kShadow,
      kPopupSelectorBackground,
      kPopupBackground,
      kPopupBorder,

      kTextComponentBackground,
      kTextComponentText,

      kRotaryArc,
      kRotaryArcDisabled,
      kRotaryArcUnselected,
      kRotaryArcUnselectedDisabled,
      kRotaryHand,
      kRotaryBody,
      kRotaryBodyBorder,

      kLinearSlider,
      kLinearSliderDisabled,
      kLinearSliderUnselected,
      kLinearSliderThumb,
      kLinearSliderThumbDisabled,

      kWidgetCenterLine,
      kWidgetPrimary1,
      kWidgetPrimary2,
      kWidgetPrimaryDisabled,
      kWidgetSecondary1,
      kWidgetSecondary2,
      kWidgetSecondaryDisabled,
      kWidgetAccent1,
      kWidgetAccent2,
      kWidgetBackground,

      kModulationMeter,
      kModulationMeterLeft,
      kModulationMeterRight,
      kModulationMeterControl,
      kModulationButtonSelected,
      kModulationButtonDragging,
      kModulationButtonUnselected,

      kIconSelectorIcon,

      kIconButtonOff,
      kIconButtonOffHover,
      kIconButtonOffPressed,
      kIconButtonOn,
      kIconButtonOnHover,
      kIconButtonOnPressed,

      kUiButton,
      kUiButtonText,
      kUiButtonHover,
      kUiButtonPressed,
      kUiActionButton,
      kUiActionButtonHover,
      kUiActionButtonPressed,

      kTextEditorBackground,
      kTextEditorBorder,
      kTextEditorCaret,
      kTextEditorSelection,

      kFinalColor
    };

    static constexpr int kNumColors = kFinalColor - kInitialColor;

    /**
     * @brief Checks if a certain ValueId should be scaled by the display ratio.
     * @param value_id The value identifier.
     * @return True if it should be scaled, false otherwise.
     */
    static bool shouldScaleValue(ValueId value_id);

    /**
     * @brief Constructs a Skin with default or loaded settings.
     *
     * If a default skin file is found, it is loaded. Otherwise, a built-in default is used.
     */
    Skin();

    /**
     * @brief Applies all component colors to a given component.
     * @param component The component to colorize.
     */
    void setComponentColors(Component* component) const;

    /**
     * @brief Applies section-specific color overrides to a component.
     * @param component The component to colorize.
     * @param section_override The section whose overrides to apply.
     * @param top_level If true, applies all top-level colors instead of overrides.
     */
    void setComponentColors(Component* component, SectionOverride section_override, bool top_level = false) const;

    /**
     * @brief Applies all default values to a SynthSection.
     * @param component The SynthSection to set values on.
     */
    void setComponentValues(SynthSection* component) const;

    /**
     * @brief Applies section-specific value overrides to a SynthSection.
     * @param component The SynthSection to set values on.
     * @param section_override The section whose overrides to apply.
     * @param top_level If true, applies all top-level values instead of overrides.
     */
    void setComponentValues(SynthSection* component, SectionOverride section_override, bool top_level = false) const;

    /**
     * @brief Sets a color for a global ColorId.
     * @param color_id The color identifier.
     * @param color The Colour to set.
     */
    void setColor(ColorId color_id, const Colour& color) { colors_[color_id - kInitialColor] = color; }

    /**
     * @brief Retrieves a globally defined color.
     * @param color_id The color identifier.
     * @return The Colour object for the given identifier.
     */
    Colour getColor(ColorId color_id) const { return colors_[color_id - kInitialColor]; }

    /**
     * @brief Retrieves a color possibly overridden by a section.
     * @param section The section override ID.
     * @param color_id The color identifier.
     * @return The Colour object for the given section and color ID.
     */
    Colour getColor(int section, ColorId color_id) const;

    /**
     * @brief Checks if a given section overrides a specific color.
     * @param section The section override ID.
     * @param color_id The color identifier.
     * @return True if overridden, false otherwise.
     */
    bool overridesColor(int section, ColorId color_id) const;

    /**
     * @brief Checks if a given section overrides a specific value.
     * @param section The section override ID.
     * @param value_id The value identifier.
     * @return True if overridden, false otherwise.
     */
    bool overridesValue(int section, ValueId value_id) const;

    /**
     * @brief Copies global skin values into a LookAndFeel instance.
     * @param look_and_feel The LookAndFeel to populate.
     */
    void copyValuesToLookAndFeel(LookAndFeel* look_and_feel) const;

    /**
     * @brief Sets a global UI value.
     * @param value_id The value identifier.
     * @param value The float value to set.
     */
    void setValue(ValueId value_id, float value) { values_[value_id] = value; }

    /**
     * @brief Gets a global UI value.
     * @param value_id The value identifier.
     * @return The current float value for that ID.
     */
    float getValue(ValueId value_id) const { return values_[value_id]; }

    /**
     * @brief Gets a value with a possible section override.
     * @param section The section override ID.
     * @param value_id The value identifier.
     * @return The overridden or default value.
     */
    float getValue(int section, ValueId value_id) const;

    /**
     * @brief Adds a color override for a given section.
     * @param section The section ID.
     * @param color_id The color to override.
     * @param color The override color.
     */
    void addOverrideColor(int section, ColorId color_id, Colour color);

    /**
     * @brief Removes a color override from a section.
     * @param section The section ID.
     * @param color_id The color ID to remove override for.
     */
    void removeOverrideColor(int section, ColorId color_id);

    /**
     * @brief Adds a value override for a given section.
     * @param section The section ID.
     * @param value_id The value to override.
     * @param value The override value.
     */
    void addOverrideValue(int section, ValueId value_id, float value);

    /**
     * @brief Removes a value override from a section.
     * @param section The section ID.
     * @param value_id The value ID to remove override for.
     */
    void removeOverrideValue(int section, ValueId value_id);

    /**
     * @brief Converts the current skin state to JSON.
     * @return The skin data in JSON form.
     */
    json stateToJson();

    /**
     * @brief Converts the current skin state to a string (JSON representation).
     * @return The skin state as a string.
     */
    String stateToString();

    /**
     * @brief Saves the current skin to a file.
     * @param destination The file to save to.
     */
    void saveToFile(File destination);

    /**
     * @brief Updates JSON data to a newer format or version if needed.
     * @param data The JSON data to update.
     * @return The updated JSON data.
     */
    json updateJson(json data);

    /**
     * @brief Loads skin state from JSON data.
     * @param skin_var The JSON data describing the skin.
     */
    void jsonToState(json skin_var);

    /**
     * @brief Loads skin state from a JSON string.
     * @param skin_string The JSON string.
     * @return True on success, false on parse errors.
     */
    bool stringToState(String skin_string);

    /**
     * @brief Loads skin state from a file.
     * @param source The file containing the skin data.
     * @return True on success, false on failure.
     */
    bool loadFromFile(File source);

    /**
     * @brief Loads a default built-in skin.
     */
    void loadDefaultSkin();

    /**
     * @brief Clears all overrides, returning to a clean default state.
     */
    void clearSkin();

  protected:
    Colour colors_[kNumColors]; ///< Array of global colors.
    float values_[kNumSkinValueIds]; ///< Array of global float values.

    std::map<ColorId, Colour> color_overrides_[kNumSectionOverrides]; ///< Per-section color overrides.
    std::map<ValueId, float> value_overrides_[kNumSectionOverrides];  ///< Per-section value overrides.
};

/**
 * @class SkinDesigner
 * @brief A DocumentWindow that allows interactive editing of the Skin.
 *
 * The SkinDesigner provides UI elements to load, save, and edit the current skin colors and values.
 * Primarily used for development and customization, it is not typically shown in a production environment.
 */
class SkinDesigner : public DocumentWindow {
  public:
    /**
     * @brief Constructs a SkinDesigner window.
     * @param skin A pointer to the Skin to be edited.
     * @param full_interface A pointer to the FullInterface for reloading and applying skin changes.
     */
    SkinDesigner(Skin* skin, FullInterface* full_interface);

    /**
     * @brief Destructor.
     */
    ~SkinDesigner();

    /**
     * @brief Handles the close button press event.
     * Closes and deletes the window.
     */
    void closeButtonPressed() override {
      delete this;
    }

  protected:
    std::unique_ptr<Component> container_; ///< Container component hosting skin editing controls.
};
