#pragma once

#include "JuceHeader.h"

class FullInterface;

/**
 * @brief A specialized ComponentBoundsConstrainer that applies border constraints
 *        and maintains aspect ratios for the Vital GUI window.
 *
 * This class ensures that when the Vital window is resized, it adheres to certain
 * borders, respects a fixed aspect ratio, and integrates with the main GUI interface
 * to perform special actions at the start and end of a resize operation.
 */
class BorderBoundsConstrainer : public ComponentBoundsConstrainer {
public:
    /**
     * @brief Constructs a new BorderBoundsConstrainer with no associated GUI
     *        and default border settings.
     */
    BorderBoundsConstrainer() : ComponentBoundsConstrainer(), gui_(nullptr) { }

    /**
     * @brief Adjusts the given bounds to enforce border constraints, maintain aspect ratio,
     *        and limit the window size based on display characteristics.
     *
     * @param bounds          The proposed new bounds of the component.
     * @param previous        The previous bounds of the component before resizing.
     * @param limits          The limits within which the component must fit.
     * @param stretching_top  True if the top edge is being dragged.
     * @param stretching_left True if the left edge is being dragged.
     * @param stretching_bottom True if the bottom edge is being dragged.
     * @param stretching_right True if the right edge is being dragged.
     *
     * This method first adjusts the bounds by subtracting the set border, then calls
     * the base class implementation to apply standard constraints. It also ensures
     * that the resulting window size does not exceed the available display area,
     * preserving the fixed aspect ratio.
     */
    virtual void checkBounds(Rectangle<int>& bounds, const Rectangle<int>& previous,
                             const Rectangle<int>& limits,
                             bool stretching_top, bool stretching_left,
                             bool stretching_bottom, bool stretching_right) override;

    /**
     * @brief Called before a resize operation begins.
     *
     * If an associated FullInterface is set, it will temporarily disable certain
     * background redraw features to improve UI responsiveness while resizing.
     */
    virtual void resizeStart() override;

    /**
     * @brief Called after a resize operation finishes.
     *
     * If an associated FullInterface is set, it saves the current window size for
     * future sessions and re-enables any previously disabled redraw features.
     */
    virtual void resizeEnd() override;

    /**
     * @brief Sets the border size that should be enforced during resizing.
     *
     * @param border The border size to apply.
     */
    void setBorder(const BorderSize<int>& border) { border_ = border; }

    /**
     * @brief Associates a FullInterface instance with this constrainer.
     *
     * @param gui A pointer to the FullInterface instance to be used.
     */
    void setGui(FullInterface* gui) { gui_ = gui; }

protected:
    /**
     * @brief A pointer to the associated GUI interface, which may be nullptr if none is set.
     */
    FullInterface* gui_;

    /**
     * @brief The border to be applied to the component bounds.
     */
    BorderSize<int> border_;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(BorderBoundsConstrainer)
};
