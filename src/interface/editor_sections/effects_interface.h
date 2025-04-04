/// @file effects_interface.h
/// @brief Declares the EffectsInterface class and related components for managing and displaying multiple effects sections.

#pragma once

#include "JuceHeader.h"
#include "default_look_and_feel.h"
#include "drag_drop_effect_order.h"
#include "open_gl_image.h"
#include "open_gl_multi_quad.h"
#include "synth_constants.h"
#include "synth_section.h"

class EffectsContainer;
class ChorusSection;
class CompressorSection;
class DelaySection;
class DistortionSection;
class DragDropEffectOrder;
class EqualizerSection;
class FilterSection;
class FlangerSection;
class PhaserSection;
class ReverbSection;

/// @class EffectsViewport
/// @brief A specialized Viewport that notifies listeners when the visible area changes (i.e., when scrolled).
class EffectsViewport : public Viewport {
public:
    /// @class Listener
    /// @brief Interface for objects that need to respond when the effects viewport is scrolled.
    class Listener {
    public:
        virtual ~Listener() { }

        /// Called when the visible area is changed (scrolled).
        /// @param position The new vertical scroll position.
        virtual void effectsScrolled(int position) = 0;
    };

    /// Adds a listener to be notified when the viewport is scrolled.
    /// @param listener The listener to add.
    void addListener(Listener* listener) { listeners_.push_back(listener); }

    /// Called when the visible area changes. Notifies listeners of scrolling.
    /// @param visible_area The new visible area.
    void visibleAreaChanged(const Rectangle<int>& visible_area) override {
        for (Listener* listener : listeners_)
            listener->effectsScrolled(visible_area.getY());

        Viewport::visibleAreaChanged(visible_area);
    }

private:
    std::vector<Listener*> listeners_; ///< Listeners to notify on scrolling.
};

/// @class EffectsInterface
/// @brief A UI component that displays and manages all effects sections and their ordering.
///
/// The EffectsInterface includes sections like Chorus, Delay, Distortion, etc., and allows reordering
/// them with DragDropEffectOrder. It also provides a scrollable viewport and a scrollbar to navigate
/// through multiple effects.
class EffectsInterface : public SynthSection, public DragDropEffectOrder::Listener,
                         public ScrollBar::Listener, public EffectsViewport::Listener {
public:
    /// @class Listener
    /// @brief Interface for objects that need to respond when effects are moved or scrolled.
    class Listener {
    public:
        virtual ~Listener() { }

        /// Called when effects are moved due to scrolling or reordering.
        virtual void effectsMoved() = 0;
    };

    /// Constructor.
    /// @param mono_modulations A map of mono modulation outputs from the synth.
    EffectsInterface(const vital::output_map& mono_modulations);

    /// Destructor.
    virtual ~EffectsInterface();

    /// Paints the background of the effects interface.
    /// @param g The graphics context.
    void paintBackground(Graphics& g) override;

    /// Paints child shadows of sections. Overridden to do nothing here.
    /// @param g The graphics context.
    void paintChildrenShadows(Graphics& g) override { }

    /// Resizes and lays out the effects sections and the viewport.
    void resized() override;

    /// Updates the background image when needed, e.g., after reordering.
    void redoBackgroundImage();

    /// Called when the effect order changes due to drag-and-drop.
    /// @param order The DragDropEffectOrder component.
    void orderChanged(DragDropEffectOrder* order) override;

    /// Called when the enabled state of an effect changes.
    /// @param order_index The index of the effect in the order.
    /// @param enabled True if the effect is now enabled, false otherwise.
    void effectEnabledChanged(int order_index, bool enabled) override;

    /// Sets the keyboard focus to this component.
    void setFocus() { grabKeyboardFocus(); }

    /// Positions the effect sections based on their order and enabled states.
    void setEffectPositions();

    /// Initializes OpenGL components for rendering background and effects visuals.
    /// @param open_gl The OpenGlWrapper for managing OpenGL state.
    void initOpenGlComponents(OpenGlWrapper& open_gl) override;

    /// Renders OpenGL components like the background image.
    /// @param open_gl The OpenGlWrapper.
    /// @param animate If true, animates transitions.
    void renderOpenGlComponents(OpenGlWrapper& open_gl, bool animate) override;

    /// Destroys OpenGL resources.
    /// @param open_gl The OpenGlWrapper.
    void destroyOpenGlComponents(OpenGlWrapper& open_gl) override;

    /// Called when the scrollbar moves.
    /// @param scroll_bar The scrollbar that moved.
    /// @param range_start The new start position.
    void scrollBarMoved(ScrollBar* scroll_bar, double range_start) override;

    /// Updates the scrollbar range after changes.
    void setScrollBarRange();

    /// Adds a listener to be notified when effects are moved.
    /// @param listener The listener to add.
    void addListener(Listener* listener) { listeners_.push_back(listener); }

    /// Called when the viewport is scrolled, updates scrollbar and notifies listeners.
    /// @param position The new vertical scroll position.
    void effectsScrolled(int position) override {
        setScrollBarRange();
        scroll_bar_->setCurrentRange(position, viewport_.getHeight());

        for (Listener* listener : listeners_)
            listener->effectsMoved();
    }

private:
    std::vector<Listener*> listeners_; ///< Listeners for movement changes.
    EffectsViewport viewport_;         ///< The viewport displaying the effects container.
    std::unique_ptr<EffectsContainer> container_; ///< Container holding all effect sections.
    OpenGlImage background_;           ///< Background image of the effects interface.
    CriticalSection open_gl_critical_section_; ///< Lock for OpenGL operations.

    std::unique_ptr<ChorusSection> chorus_section_;
    std::unique_ptr<CompressorSection> compressor_section_;
    std::unique_ptr<DelaySection> delay_section_;
    std::unique_ptr<DistortionSection> distortion_section_;
    std::unique_ptr<EqualizerSection> equalizer_section_;
    std::unique_ptr<FlangerSection> flanger_section_;
    std::unique_ptr<PhaserSection> phaser_section_;
    std::unique_ptr<ReverbSection> reverb_section_;
    std::unique_ptr<FilterSection> filter_section_;
    std::unique_ptr<DragDropEffectOrder> effect_order_;
    std::unique_ptr<OpenGlScrollBar> scroll_bar_;

    SynthSection* effects_list_[vital::constants::kNumEffects]; ///< Array of pointers to the effects in order.

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(EffectsInterface)
};
