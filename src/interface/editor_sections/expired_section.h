#pragma once

#include "JuceHeader.h"
#include "overlay.h"
#include "open_gl_image_component.h"

/**
 * @class ExpiredSection
 * @brief A section overlay that displays an expiration message for a beta version of the plugin.
 *
 * This class represents an overlay section indicating that the current version of the software has expired.
 * It provides a message and a hyperlink for the user to download a newer version.
 */
class ExpiredSection : public Overlay {
public:
    /**
     * @brief The fixed width of the expired section overlay.
     */
    static constexpr int kExpiredWidth = 340;

    /**
     * @brief The fixed height of the expired section overlay.
     */
    static constexpr int kExpiredHeight = 85;

    /**
     * @brief The horizontal padding within the overlay.
     */
    static constexpr int kPaddingX = 25;

    /**
     * @brief The vertical padding within the overlay.
     */
    static constexpr int kPaddingY = 20;

    /**
     * @brief The height of the clickable button area (e.g., hyperlink text).
     */
    static constexpr int kButtonHeight = 30;

    /**
     * @brief Constructs an ExpiredSection overlay.
     * @param name The name of this component.
     */
    ExpiredSection(String name);

    /**
     * @brief Destructor.
     */
    virtual ~ExpiredSection() = default;

    /**
     * @brief Called when the component is resized.
     *
     * Positions and sizes internal components accordingly.
     */
    void resized() override;

    /**
     * @brief Sets the visibility of this overlay.
     * @param should_be_visible True to make visible, false to hide.
     */
    void setVisible(bool should_be_visible) override;

    /**
     * @brief Computes the rectangle area occupied by the expired message section.
     * @return A Rectangle<int> defining the area of the expired section.
     */
    Rectangle<int> getExpiredRect();

private:
    /**
     * @brief The underlying body component, drawn as a rounded rectangle.
     */
    OpenGlQuad body_;

    /**
     * @brief The text component that displays the expiration message.
     */
    std::unique_ptr<PlainTextComponent> text_;

    /**
     * @brief A hyperlink that redirects users to a location where they can download a new version.
     */
    std::unique_ptr<class OpenGlHyperlink> link_;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(ExpiredSection)
};
