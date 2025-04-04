#pragma once

#include "JuceHeader.h"

#include "open_gl_component.h"
#include "open_gl_multi_quad.h"
#include "synth_section.h"
#include "wavetable_playhead.h"
#include "wavetable_organizer.h"

/**
 * @class WavetableComponentOverlay
 * @brief A base overlay component for editing and interacting with a wavetable component's parameters.
 *
 * The WavetableComponentOverlay class provides a structured overlay UI for wavetable editing.
 * It supports customizable sections and line dividers, along with titles and associated controls.
 * Derived classes can add their own controls (such as sliders and buttons) and update their positions
 * and visuals according to the wavetable keyframes selected.
 */
class WavetableComponentOverlay : public SynthSection,
                                  public WavetablePlayhead::Listener,
                                  public WavetableOrganizer::Listener {
  public:
    /// Maximum grid lines used by some overlays.
    static constexpr int kMaxGrid = 16;

    /// Ratio constants for layout and sizing.
    static constexpr float kTitleHeightForWidth = 0.1f;
    static constexpr float kWidgetHeightForWidth = 0.08f;
    static constexpr float kShadowPercent = 0.1f;
    static constexpr float kDividerPoint = 0.44f;
    static constexpr float kTitleHeightRatio = 0.4f;

    /**
     * @class ControlsBackground
     * @brief A background component with lines and titles for the overlay's control section.
     *
     * The ControlsBackground class draws a styled background with configurable line dividers
     * and titles for control groupings. This is used within WavetableComponentOverlay to
     * segment and label controls such as sliders and selectors.
     */
    class ControlsBackground : public SynthSection {
      public:
        /// Maximum possible lines for control divisions.
        static constexpr int kMaxLines = 16;

        /**
         * @brief Constructs a ControlsBackground component.
         *
         * Initializes OpenGL components for backgrounds, borders, lines, and titles.
         */
        ControlsBackground() : SynthSection("background"), background_(Shaders::kRoundedRectangleFragment),
                               border_(Shaders::kRoundedRectangleBorderFragment),
                               lines_(kMaxLines, Shaders::kColorFragment),
                               title_backgrounds_(kMaxLines + 1, Shaders::kColorFragment) {
          addOpenGlComponent(&background_);
          addOpenGlComponent(&border_);
          addOpenGlComponent(&lines_);
          addOpenGlComponent(&title_backgrounds_);

          background_.setTargetComponent(this);
          border_.setTargetComponent(this);
          lines_.setTargetComponent(this);
          title_backgrounds_.setTargetComponent(this);

          for (int i = 0; i <= kMaxLines; ++i) {
            title_texts_[i] = std::make_unique<PlainTextComponent>("text", "");
            addOpenGlComponent(title_texts_[i].get());
            title_texts_[i]->setActive(false);
            title_texts_[i]->setFontType(PlainTextComponent::kLight);
          }
        }

        /**
         * @brief Clears all line divider positions.
         */
        void clearLines() { line_positions_.clear(); setPositions(); }

        /**
         * @brief Clears all control section titles.
         */
        void clearTitles() { titles_.clear(); setPositions(); }

        /**
         * @brief Adds a vertical line divider at the given position.
         * @param position The x-position (in pixels) of the vertical line.
         */
        void addLine(int position) { line_positions_.push_back(position); setPositions(); }

        /**
         * @brief Adds a title string for the next control section.
         * @param title The title text for the control section.
         */
        void addTitle(const std::string& title) { titles_.push_back(title); setPositions(); }

        /**
         * @brief Updates all OpenGL components and text positions after changes to lines or titles.
         */
        void setPositions();

      private:
        OpenGlQuad background_;
        OpenGlQuad border_;
        OpenGlMultiQuad lines_;
        OpenGlMultiQuad title_backgrounds_;
        std::unique_ptr<PlainTextComponent> title_texts_[kMaxLines + 1];
        std::vector<int> line_positions_;
        std::vector<std::string> titles_;
    };

    /**
     * @class Listener
     * @brief A listener interface for receiving changes to the wavetable overlay.
     *
     * Implement this interface to receive notifications when a frame finishes editing
     * or when a frame has changed.
     */
    class Listener {
      public:
        virtual ~Listener() { }

        /**
         * @brief Called when the user has finished editing the current frame.
         */
        virtual void frameDoneEditing() = 0;

        /**
         * @brief Called when the current frame is changed or updated.
         */
        virtual void frameChanged() = 0;
    };

    /**
     * @brief Constructs a WavetableComponentOverlay.
     * @param name A name identifier for this overlay.
     *
     * Initializes the overlay with a background control section.
     */
    WavetableComponentOverlay(String name) : SynthSection(name), current_component_(nullptr),
                                             controls_width_(0), initialized_(false), padding_(0) {
      setInterceptsMouseClicks(false, true);
      addSubSection(&controls_background_);
      controls_background_.setAlwaysOnTop(true);
    }

    /**
     * @brief Deleted default constructor.
     */
    WavetableComponentOverlay() = delete;

    /**
     * @brief Custom paint method for background.
     * @param g Graphics context to paint into.
     */
    void paintBackground(Graphics& g) override { paintChildrenBackgrounds(g); }

    /**
     * @brief Called when the wavetable playhead moves, but default does nothing.
     * @param position The new playhead position.
     */
    void playheadMoved(int position) override { }

    /**
     * @brief Sets the editing bounds within which controls and titles are placed.
     * @param bounds The rectangular area used for editing UI components.
     */
    virtual void setEditBounds(Rectangle<int> bounds);

    /**
     * @brief Optionally set bounds for time-domain editing UI.
     * @param bounds The rectangular area used for time-domain UI.
     * @return True if handled, false otherwise.
     */
    virtual bool setTimeDomainBounds(Rectangle<int> bounds) { return false; }

    /**
     * @brief Optionally set bounds for frequency-amplitude editing UI.
     * @param bounds The rectangular area used for frequency-amplitude UI.
     * @return True if handled, false otherwise.
     */
    virtual bool setFrequencyAmplitudeBounds(Rectangle<int> bounds) { return false; }

    /**
     * @brief Optionally set bounds for phase editing UI.
     * @param bounds The rectangular area used for phase UI.
     * @return True if handled, false otherwise.
     */
    virtual bool setPhaseBounds(Rectangle<int> bounds) { return false; }

    /**
     * @brief Resets the overlay, clearing any associated component.
     */
    void resetOverlay();

    /**
     * @brief Initializes OpenGL components.
     * @param open_gl The OpenGL wrapper.
     */
    void initOpenGlComponents(OpenGlWrapper& open_gl) override {
      SynthSection::initOpenGlComponents(open_gl);
      initialized_ = true;
    }

    /**
     * @brief Checks if the overlay has been initialized.
     * @return True if OpenGL initialization is complete.
     */
    bool initialized() { return initialized_; }

    /**
     * @brief Adds a listener for frame changes.
     * @param listener The listener to add.
     */
    void addFrameListener(Listener* listener) {
      listeners_.push_back(listener);
    }

    /**
     * @brief Removes a frame listener.
     * @param listener The listener to remove.
     */
    void removeListener(Listener* listener) {
      listeners_.erase(std::find(listeners_.begin(), listeners_.end(), listener));
    }

    /**
     * @brief Sets whether to scale values like frequency display (optional override).
     * @param scale True if scaling is enabled.
     */
    virtual void setPowerScale(bool scale) { }

    /**
     * @brief Sets the frequency zoom factor (optional override).
     * @param zoom The new zoom factor.
     */
    virtual void setFrequencyZoom(float zoom) { }

    /**
     * @brief Sets padding around controls and triggers a repaint.
     * @param padding The new padding value.
     */
    void setPadding(int padding) { padding_ = padding; repaint(); }

    /**
     * @brief Gets the current padding value.
     * @return The current padding.
     */
    int getPadding() { return padding_; }

    /**
     * @brief Sets the WavetableComponent that this overlay is editing.
     * @param component The wavetable component to associate with this overlay.
     */
    void setComponent(WavetableComponent* component);

    /**
     * @brief Gets the currently associated WavetableComponent.
     * @return The currently associated component, or nullptr if none.
     */
    WavetableComponent* getComponent() { return current_component_; }

  protected:
    /**
     * @brief Sets the total width for controls in the overlay.
     * @param width The total width in pixels for the controls.
     */
    void setControlsWidth(int width) { controls_width_ = width; repaint(); }

    /**
     * @brief Notifies listeners that a change has occurred to the frame.
     * @param mouse_up True if the mouse is released, finalizing the edit.
     */
    void notifyChanged(bool mouse_up);

    /**
     * @brief Gets the title height based on ratio and current edit bounds.
     * @return The computed title height in pixels.
     */
    float getTitleHeight();

    /**
     * @brief Gets the x position of a divider line.
     * @return The computed divider x position in pixels.
     */
    int getDividerX();

    /**
     * @brief Gets the widget height for controls.
     * @return The computed widget height.
     */
    int getWidgetHeight();

    /**
     * @brief Gets the widget padding.
     * @return The computed widget padding.
     */
    int getWidgetPadding();

    WavetableComponent* current_component_;
    ControlsBackground controls_background_;
    std::vector<Listener*> listeners_;
    Rectangle<int> edit_bounds_;
    int controls_width_;
    bool initialized_;
    int padding_;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(WavetableComponentOverlay)
};

