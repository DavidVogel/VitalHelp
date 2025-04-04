#pragma once

#include "JuceHeader.h"
#include "value.h"
#include "synth_parameters.h"

/**
 * @class ValueBridge
 * @brief A parameter bridge that connects a @c vital::Value to an AudioProcessorParameter, allowing
 *        the host to manipulate and automate internal values within the Vital synth engine.
 *
 * This class extends JUCE's @c AudioProcessorParameter to expose internal Vital parameters as
 * host-facing automation parameters. It handles conversions between normalized (0.0 to 1.0)
 * parameter values and the internal engine's value ranges, as well as skewing/scaling functions
 * (e.g., exponential, quadratic) that convert between user-facing displays and engine values.
 */
class ValueBridge : public AudioProcessorParameter {
  public:
    /**
     * @class Listener
     * @brief An interface for receiving parameter change notifications from the ValueBridge.
     *
     * Implementors of this interface can register with a ValueBridge to be notified whenever
     * the parameter changes. This allows internal synth parameters to remain in sync with
     * externally automated parameters.
     */
    class Listener {
      public:
        /**
         * @brief Virtual destructor.
         */
        virtual ~Listener() { }

        /**
         * @brief Called when the parameter value changes.
         *
         * @param name The name of the parameter.
         * @param value The new value of the parameter in the engine's internal range.
         */
        virtual void parameterChanged(std::string name, vital::mono_float value) = 0;
    };

    /**
     * @brief Deleted default constructor.
     */
    ValueBridge() = delete;

    /**
     * @brief Constructs a ValueBridge to expose a Vital parameter as a host-automatable parameter.
     *
     * @param name The parameter's name (used to look up details and for automation labeling).
     * @param value A pointer to the internal Vital parameter value object.
     */
    ValueBridge(std::string name, vital::Value* value) :
        AudioProcessorParameter(), name_(name), value_(value), listener_(nullptr),
        source_changed_(false) {
      details_ = vital::Parameters::getDetails(name);
      span_ = details_.max - details_.min;
      if (details_.value_scale == vital::ValueDetails::kIndexed)
        span_ = std::round(span_);
    }

    /**
     * @brief Gets the current normalized (0.0 to 1.0) value of the parameter.
     *
     * @return The parameter value as a normalized float.
     */
    float getValue() const override {
      return convertToPluginValue(value_->value());
    }

    /**
     * @brief Sets the parameter value from a normalized (0.0 to 1.0) float.
     *
     * Notifies the listener if one is set. This method updates the internal engine value
     * by converting from the normalized range.
     *
     * @param value The new normalized value of the parameter.
     */
    void setValue(float value) override {
      if (listener_ && !source_changed_) {
        source_changed_ = true;
        vital::mono_float synth_value = convertToEngineValue(value);
        listener_->parameterChanged(name_.toStdString(), synth_value);
        source_changed_ = false;
      }
    }

    /**
     * @brief Sets a listener to receive parameter change callbacks.
     *
     * @param listener A pointer to a Listener implementation, or nullptr to remove the listener.
     */
    void setListener(Listener* listener) {
      listener_ = listener;
    }

    /**
     * @brief Returns the default normalized value of this parameter.
     *
     * @return The default normalized value.
     */
    float getDefaultValue() const override {
      return convertToPluginValue(details_.default_value);
    }

    /**
     * @brief Returns the display name of this parameter, truncated to a given length.
     *
     * @param maximumStringLength The maximum length of the returned string.
     * @return The truncated parameter name.
     */
    String getName(int maximumStringLength) const override {
      return String(details_.display_name).substring(0, maximumStringLength);
    }

    /**
     * @brief Returns the label (unit) associated with this parameter.
     *
     * For this parameter bridge, it's typically an empty string.
     *
     * @return The label of the parameter.
     */
    String getLabel() const override {
      return "";
    }

    /**
     * @brief Converts a normalized value into a user-facing text string.
     *
     * Converts the provided normalized value back into the engine's range, potentially
     * applying skewing or exponential transforms, and formats it as a string with units.
     * If a string lookup table is available, it is used.
     *
     * @param value The normalized parameter value.
     * @param maximumStringLength The maximum allowed length of the returned string.
     * @return A string representation of the parameter value.
     */
    String getText(float value, int maximumStringLength) const override {
      float adjusted = convertToEngineValue(value);
      String result = "";
      if (details_.string_lookup)
        result = details_.string_lookup[std::max<int>(0, std::min(adjusted, details_.max))];
      else {
        float display_value = details_.display_multiply * skewValue(adjusted) + details_.post_offset;
        result = String(display_value) + details_.display_units;
      }
      return result.substring(0, maximumStringLength).trim();
    }

    /**
     * @brief Converts a user-facing string back into a normalized parameter value.
     *
     * This reverses the skewing/scaling and units applied in @c getText().
     *
     * @param text The string representing a parameter value.
     * @return The corresponding normalized parameter value.
     */
    float getValueForText(const String &text) const override {
      return unskewValue(text.getFloatValue() / details_.display_multiply);
    }

    /**
     * @brief Indicates whether this parameter can be automated by the host.
     *
     * @return True if the parameter is automatable.
     */
    bool isAutomatable() const override {
      return true;
    }

    /**
     * @brief Returns the number of discrete steps this parameter has, if any.
     *
     * If the parameter is discrete and indexed, the number of steps is 1 + span.
     * Otherwise, defer to the base class.
     *
     * @return The number of steps as an integer.
     */
    int getNumSteps() const override {
      if (isDiscrete())
        return 1 + (int)span_;
      return AudioProcessorParameter::getNumSteps();
    }

    /**
     * @brief Checks if this parameter is discrete (indexed steps) or continuous.
     *
     * @return True if the parameter is discrete, false otherwise.
     */
    bool isDiscrete() const override {
      static constexpr int kMaxIndexedSteps = 300;
      return details_.value_scale == vital::ValueDetails::kIndexed && span_ < kMaxIndexedSteps;
    }

    /**
     * @brief Checks if this parameter is essentially a boolean (on/off) parameter.
     *
     * @return True if the parameter has only two discrete states, false otherwise.
     */
    bool isBoolean() const override {
      return isDiscrete() && span_ == 1.0f;
    }

    /**
     * @brief Converts the internal engine value to a normalized value from 0.0 to 1.0.
     *
     * @param synth_value The engine's parameter value.
     * @return A normalized value from 0.0 to 1.0.
     */
    float convertToPluginValue(vital::mono_float synth_value) const {
      return (synth_value - details_.min) / span_;
    }

    /**
     * @brief Converts a normalized (0.0 to 1.0) parameter value back into the engine's range.
     *
     * If the parameter is indexed, the returned value is rounded to the nearest integer.
     *
     * @param plugin_value The normalized parameter value.
     * @return The engine's parameter value.
     */
    float convertToEngineValue(vital::mono_float plugin_value) const {
      float value = plugin_value * span_ + details_.min;

      if (details_.value_scale == vital::ValueDetails::kIndexed)
        return std::round(value);

      return value;
    }

    /**
     * @brief Sets the parameter value and notifies the host, preventing recursive updates.
     *
     * @param new_value The new normalized parameter value.
     */
    void setValueNotifyHost(float new_value) {
      if (!source_changed_) {
        source_changed_ = true;
        setValueNotifyingHost(new_value);
        source_changed_ = false;
      }
    }

  private:
    /**
     * @brief Returns the current parameter value in skewed form.
     *
     * @return The skewed parameter value.
     */
    float getSkewedValue() const {
      return skewValue(value_->value());
    }

    /**
     * @brief Applies a skewing/scaling transformation to a parameter value.
     *
     * Depending on @c details_.value_scale, this may apply quadratic, cubic, quartic,
     * exponential, or square root transformations. For exponential transformations,
     * invert can also be applied.
     *
     * @param value The original parameter value.
     * @return The transformed parameter value.
     */
    float skewValue(float value) const {
      switch (details_.value_scale) {
        case vital::ValueDetails::kQuadratic:
          return value * value;
        case vital::ValueDetails::kCubic:
          return value * value * value;
        case vital::ValueDetails::kQuartic:
          value *= value;
          return value * value;
        case vital::ValueDetails::kExponential:
          if (details_.display_invert)
            return 1.0f / powf(2.0f, value);
          return powf(2.0f, value);
        case vital::ValueDetails::kSquareRoot:
          return sqrtf(value);
        default:
          return value;
      }
    }

    /**
     * @brief Reverses the skewing/transformation applied by @c skewValue().
     *
     * @param value The skewed parameter value.
     * @return The original unskewed parameter value.
     */
    float unskewValue(float value) const {
      switch (details_.value_scale) {
        case vital::ValueDetails::kQuadratic:
          return sqrtf(value);
        case vital::ValueDetails::kCubic:
          return powf(value, 1.0f / 3.0f);
        case vital::ValueDetails::kQuartic:
          return powf(value, 1.0f / 4.0f);
        case vital::ValueDetails::kExponential:
          if (details_.display_invert)
            return log2(1.0f / value);
          return log2(value);
        default:
          return value;
      }
    }

    String name_;                 ///< The name of this parameter.
    vital::ValueDetails details_; ///< Details about the parameter (range, scale, etc.).
    vital::mono_float span_;      ///< The span (max - min) of this parameter's engine values.
    vital::Value* value_;         ///< Pointer to the underlying Vital parameter.
    Listener* listener_;          ///< Optional listener for parameter change events.
    bool source_changed_;         ///< Flag to prevent recursive updates when changing the value.

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(ValueBridge)
};

