#pragma once

#include "JuceHeader.h"
#include "synth_section.h"

class MidiKeyboard;

/**
 * @class KeyboardInterface
 * @brief A UI section of the synthesizer interface that displays and handles a virtual MIDI keyboard.
 *
 * This class integrates a virtual MIDI keyboard into the synth’s user interface,
 * allowing the user to visualize and interact with MIDI note input. It inherits
 * from SynthSection and uses JUCE's MIDI handling and drawing routines.
 */
class KeyboardInterface : public SynthSection {
public:
    /**
     * @brief Constructs a new KeyboardInterface.
     *
     * @param keyboard_state A pointer to a MidiKeyboardState object that
     * represents the current MIDI keyboard state.
     */
    KeyboardInterface(MidiKeyboardState* keyboard_state);

    /**
     * @brief Destructor.
     */
    ~KeyboardInterface();

    /**
     * @brief Paints the background of the keyboard interface.
     *
     * @param g The JUCE Graphics context to use for drawing.
     *
     * This method is called as part of the component's paint routine.
     */
    void paintBackground(Graphics& g) override;

    /**
     * @brief Paints the shadow for the background (if the section is active).
     *
     * @param g The JUCE Graphics context to use for drawing.
     *
     * Overrides the default SynthSection method to draw a tab shadow
     * if the interface is active.
     */
    void paintBackgroundShadow(Graphics& g) override { if (isActive()) paintTabShadow(g); }

    /**
     * @brief Called when the component is resized.
     *
     * Recalculates the layout of the contained MIDI keyboard.
     */
    void resized() override;

    /**
     * @brief Sets the focus to this component, ensuring keyboard events are
     * directed here.
     */
    void setFocus() { grabKeyboardFocus(); }

private:
    /**
     * @brief The virtual MIDI keyboard component.
     *
     * Owned by this class and handles graphical representation
     * and interaction with MIDI notes.
     */
    std::unique_ptr<MidiKeyboard> keyboard_;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(KeyboardInterface)
};
