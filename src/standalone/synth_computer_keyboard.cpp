#include "synth_computer_keyboard.h"

#include "synth_constants.h"
#include "sound_engine.h"

SynthComputerKeyboard::SynthComputerKeyboard(vital::SoundEngine* synth,
                                             MidiKeyboardState* keyboard_state) {
  synth_ = synth;
  keyboard_state_ = keyboard_state;
  computer_keyboard_offset_ = vital::kDefaultKeyboardOffset;
  layout_ = vital::kDefaultKeyboard;
  up_key_ = vital::kDefaultKeyboardOctaveUp;
  down_key_ = vital::kDefaultKeyboardOctaveDown;
}

SynthComputerKeyboard::~SynthComputerKeyboard() {
}

void SynthComputerKeyboard::changeKeyboardOffset(int new_offset) {
  // Turn off all currently active notes at the old offset
  for (int i = 0; i < layout_.length(); ++i) {
    int note = computer_keyboard_offset_ + i;
    keyboard_state_->noteOff(kKeyboardMidiChannel, note, 0.5f);
    keys_pressed_.erase(layout_[i]);
  }

  // Clamp the new offset to a valid MIDI note range
  int max = (vital::kMidiSize / vital::kNotesPerOctave - 1) * vital::kNotesPerOctave;
  computer_keyboard_offset_ = vital::utils::iclamp(new_offset, 0, max);
}

bool SynthComputerKeyboard::keyPressed(const KeyPress &key, Component *origin) {
  // This method doesn't handle note-on logic directly, it's handled in keyStateChanged.
  // Return false to indicate the event was not consumed here.
  return false;
}

bool SynthComputerKeyboard::keyStateChanged(bool isKeyDown, Component *origin) {
  bool consumed = false;
  // Check through each character in the keyboard layout
  for (int i = 0; i < layout_.length(); ++i) {
    int note = computer_keyboard_offset_ + i;
    ModifierKeys modifiers = ModifierKeys::getCurrentModifiersRealtime();

    // If a key mapped to a note is currently down and wasn't previously registered as pressed:
    if (KeyPress::isKeyCurrentlyDown(layout_[i]) &&
        !keys_pressed_.count(layout_[i]) && isKeyDown && !modifiers.isCommandDown()) {
      keys_pressed_.insert(layout_[i]);
      keyboard_state_->noteOn(kKeyboardMidiChannel, note, 1.0f);
    }
    // If a key mapped to a note was previously pressed but is now released:
    else if (!KeyPress::isKeyCurrentlyDown(layout_[i]) && keys_pressed_.count(layout_[i])) {
      keys_pressed_.erase(layout_[i]);
      keyboard_state_->noteOff(kKeyboardMidiChannel, note, 0.5f);
    }
    consumed = true;
  }

  // Handle octave down key
  if (KeyPress::isKeyCurrentlyDown(down_key_)) {
    if (!keys_pressed_.count(down_key_)) {
      keys_pressed_.insert(down_key_);
      changeKeyboardOffset(computer_keyboard_offset_ - vital::kNotesPerOctave);
      consumed = true;
    }
  }
  else
    keys_pressed_.erase(down_key_);

  // Handle octave up key
  if (KeyPress::isKeyCurrentlyDown(up_key_)) {
    if (!keys_pressed_.count(up_key_)) {
      keys_pressed_.insert(up_key_);
      changeKeyboardOffset(computer_keyboard_offset_ + vital::kNotesPerOctave);
      consumed = true;
    }
  }
  else
    keys_pressed_.erase(up_key_);

  // Handle space key to resync synth time
  if (KeyPress::isKeyCurrentlyDown(KeyPress::spaceKey)) {
    if (!keys_pressed_.count(' ')) {
      keys_pressed_.insert(' ');
      synth_->correctToTime(0.0);
      consumed = true;
    }
  }
  else
    keys_pressed_.erase(' ');

  return consumed;
}
