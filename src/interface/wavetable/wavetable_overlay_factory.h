#pragma once

#include "JuceHeader.h"
#include "wavetable_component_factory.h"

class WavetableComponentOverlay;

/**
 * @class WavetableOverlayFactory
 * @brief A factory class for creating and configuring overlay components for wavetable editing.
 *
 * The WavetableOverlayFactory is responsible for instantiating the appropriate overlay component
 * based on the type of a WavetableComponent. It also supports setting the overlay's owner after
 * creation to properly connect the overlay with the underlying wavetable component for editing.
 */
class WavetableOverlayFactory {
public:
    /**
     * @brief Create an overlay for a given wavetable component type.
     * @param component_type The component type for which the overlay is needed.
     * @return A pointer to a newly created WavetableComponentOverlay, or nullptr if invalid type.
     *
     * This function constructs a specialized overlay class instance corresponding to the provided
     * WavetableComponentFactory::ComponentType. The caller is responsible for managing the lifetime
     * of the returned overlay.
     */
    static WavetableComponentOverlay* createOverlay(WavetableComponentFactory::ComponentType component_type);

    /**
     * @brief Set the owner WavetableComponent of an already created overlay.
     * @param overlay The overlay whose owner should be set.
     * @param owner The WavetableComponent that this overlay will edit.
     *
     * After creating an overlay with createOverlay(), this function configures it with the correct
     * owner. The owner is the wavetable component that the overlay will modify, allowing the overlay
     * to display and manipulate the component's parameters.
     */
    static void setOverlayOwner(WavetableComponentOverlay* overlay, WavetableComponent* owner);

private:
    /**
     * @brief Private constructor to prevent instantiation, as this class only provides static methods.
     */
    WavetableOverlayFactory() { }
};
