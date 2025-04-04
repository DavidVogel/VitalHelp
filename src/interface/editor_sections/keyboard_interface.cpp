#include "keyboard_interface.h"

#include "skin.h"
#include "midi_keyboard.h"

/**
 * @brief Constructs the KeyboardInterface and initializes the MIDI keyboard.
 *
 * @param keyboard_state A pointer to a MidiKeyboardState object that
 * represents the current MIDI keyboard state.
 */
KeyboardInterface::KeyboardInterface(MidiKeyboardState* keyboard_state) : SynthSection("keyboard") {
  keyboard_ = std::make_unique<MidiKeyboard>(*keyboard_state);
  addOpenGlComponent(keyboard_.get());

  setOpaque(false);
  setSkinOverride(Skin::kKeyboard);
}

/**
 * @brief Destructor for KeyboardInterface.
 */
KeyboardInterface::~KeyboardInterface() { }

/**
 * @brief Paints the background of the keyboard section.
 *
 * @param g The JUCE Graphics context used for drawing.
 */
void KeyboardInterface::paintBackground(Graphics& g) {
  paintBody(g);
  paintChildrenBackgrounds(g);
}

/**
 * @brief Resizes the keyboard to fit the current bounds.
 */
void KeyboardInterface::resized() {
  keyboard_->setBounds(getLocalBounds());
  SynthSection::resized();
}
