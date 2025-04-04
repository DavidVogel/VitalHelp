#include "border_bounds_constrainer.h"
#include "full_interface.h"
#include "load_save.h"
#include "synth_gui_interface.h"

void BorderBoundsConstrainer::checkBounds(Rectangle<int>& bounds, const Rectangle<int>& previous,
                                          const Rectangle<int>& limits,
                                          bool stretching_top, bool stretching_left,
                                          bool stretching_bottom, bool stretching_right) {
  // Apply the defined border before checking the standard constraints.
  border_.subtractFrom(bounds);
  double aspect_ratio = getFixedAspectRatio();

  // Let the base class handle initial constraint checks.
  ComponentBoundsConstrainer::checkBounds(bounds, previous, limits,
                                          stretching_top, stretching_left,
                                          stretching_bottom, stretching_right);

  // Get the total display area and adjust for window frame size if a GUI is available.
  Rectangle<int> display_area = Desktop::getInstance().getDisplays().getTotalBounds(true);
  if (gui_) {
    ComponentPeer* peer = gui_->getPeer();
    if (peer)
      peer->getFrameSize().subtractFrom(display_area);
  }

  // Ensure the window doesn't exceed the display area width.
  if (display_area.getWidth() < bounds.getWidth()) {
    int new_width = display_area.getWidth();
    int new_height = std::round(new_width / aspect_ratio);
    bounds.setWidth(new_width);
    bounds.setHeight(new_height);
  }

  // Ensure the window doesn't exceed the display area height.
  if (display_area.getHeight() < bounds.getHeight()) {
    int new_height = display_area.getHeight();
    int new_width = std::round(new_height * aspect_ratio);
    bounds.setWidth(new_width);
    bounds.setHeight(new_height);
  }

  // Reapply the border to the adjusted bounds.
  border_.addTo(bounds);
}

void BorderBoundsConstrainer::resizeStart() {
  // Temporarily disable background redraw for smoother resizing if GUI is present.
  if (gui_)
    gui_->enableRedoBackground(false);
}

void BorderBoundsConstrainer::resizeEnd() {
  // Once resizing is complete, save the new window size and re-enable redraw if GUI is present.
  if (gui_) {
    LoadSave::saveWindowSize(gui_->getWidth() / (1.0f * vital::kDefaultWindowWidth));
    gui_->enableRedoBackground(true);
  }
}
