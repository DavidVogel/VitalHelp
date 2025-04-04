/// @file envelope_section.h
/// @brief Declares the EnvelopeSection class, which provides a UI for configuring ADSR-type envelopes.

#pragma once

#include "JuceHeader.h"
#include "synth_section.h"

class EnvelopeEditor;
class SynthSlider;

/// @class DragMagnifyingGlass
/// @brief A draggable magnification control for the envelope editor.
///
/// The DragMagnifyingGlass is an OpenGlShapeButton shaped like a magnifying glass. Dragging it allows the user
/// to zoom in or out on the envelope editor area. Double-clicking resets the zoom.
class DragMagnifyingGlass : public OpenGlShapeButton {
public:
    /// @class Listener
    /// @brief Interface for objects that need to respond to magnification changes.
    class Listener {
    public:
        virtual ~Listener() { }
        /// Called when the magnifying glass is dragged.
        /// @param delta The movement delta of the drag.
        virtual void magnifyDragged(Point<float> delta) = 0;
        /// Called when the magnifying glass is double-clicked.
        virtual void magnifyDoubleClicked() = 0;
    };

    /// Constructor.
    DragMagnifyingGlass();

    /// Called when the mouse button is pressed down. Hides the cursor and enters unbounded movement if possible.
    /// @param e The mouse event.
    void mouseDown(const MouseEvent& e) override;

    /// Called when the mouse button is released. Restores the cursor and disables unbounded movement if it was enabled.
    /// @param e The mouse event.
    void mouseUp(const MouseEvent& e) override;

    /// Called while the mouse is dragged. Notifies listeners with the movement delta.
    /// @param e The mouse event.
    void mouseDrag(const MouseEvent& e) override;

    /// Called when the magnifying glass is double-clicked. Notifies listeners to reset zoom.
    /// @param e The mouse event.
    void mouseDoubleClick(const MouseEvent& e) override;

    /// Adds a listener to respond to drag and double-click events.
    /// @param listener The listener to add.
    void addListener(Listener* listener) { listeners_.push_back(listener); }

private:
    Point<float> last_position_;       ///< Last mouse position recorded.
    Point<int> mouse_down_position_;   ///< Screen position of mouse-down for restoring when unbounded movement ends.
    std::vector<Listener*> listeners_; ///< Listeners to notify of drag and double-click events.
};

/// @class EnvelopeSection
/// @brief A UI section for configuring and visualizing ADSR envelopes.
///
/// The EnvelopeSection provides sliders for ADSR parameters (Delay, Attack, Hold, Decay, Sustain, Release) and
/// an EnvelopeEditor to visualize and adjust the envelope shape. A DragMagnifyingGlass control allows zooming
/// and panning the envelope view for fine adjustments.
class EnvelopeSection : public SynthSection, public DragMagnifyingGlass::Listener {
public:
    /// Constructor.
    /// @param name The name of this envelope section.
    /// @param value_prepend A string prefix for parameter names.
    /// @param mono_modulations A map of mono modulation outputs.
    /// @param poly_modulations A map of poly modulation outputs.
    EnvelopeSection(String name, std::string value_prepend,
                    const vital::output_map& mono_modulations,
                    const vital::output_map& poly_modulations);

    /// Destructor.
    virtual ~EnvelopeSection();

    /// Paints the background and labels for the envelope parameters.
    /// @param g The graphics context.
    void paintBackground(Graphics& g) override;

    /// Resizes and lays out child components, including the envelope editor and sliders.
    void resized() override;

    /// Resets the envelope editor to its default positions.
    void reset() override;

    /// Called when the magnifying glass is dragged. Adjusts envelope zoom accordingly.
    /// @param delta The drag movement delta.
    void magnifyDragged(Point<float> delta) override;

    /// Called when the magnifying glass is double-clicked. Resets envelope zoom.
    void magnifyDoubleClicked() override;

private:
    std::unique_ptr<EnvelopeEditor> envelope_;           ///< The envelope editor for visualizing ADSR shapes.
    std::unique_ptr<SynthSlider> delay_;                 ///< Delay time slider.
    std::unique_ptr<SynthSlider> attack_;                ///< Attack time slider.
    std::unique_ptr<SynthSlider> attack_power_;          ///< Attack curve power slider.
    std::unique_ptr<SynthSlider> hold_;                  ///< Hold time slider.
    std::unique_ptr<SynthSlider> decay_;                 ///< Decay time slider.
    std::unique_ptr<SynthSlider> decay_power_;           ///< Decay curve power slider.
    std::unique_ptr<SynthSlider> sustain_;               ///< Sustain level slider.
    std::unique_ptr<SynthSlider> release_;               ///< Release time slider.
    std::unique_ptr<SynthSlider> release_power_;         ///< Release curve power slider.
    std::unique_ptr<DragMagnifyingGlass> drag_magnifying_glass_; ///< Magnifying control for zooming envelope view.

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(EnvelopeSection)
};
