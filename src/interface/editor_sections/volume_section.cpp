#include "volume_section.h"
#include "peak_meter_viewer.h"
#include "fonts.h"
#include "skin.h"
#include "synth_slider.h"
#include "synth_parameters.h"

/**
 * @class VolumeSlider
 * @brief A custom slider for controlling the output volume.
 *
 * The VolumeSlider extends SynthSlider with a custom paint routine that displays an arrow marker
 * indicating the current volume level. The slider is linear and represented as a vertical bar.
 */
class VolumeSlider : public SynthSlider {
  public:
    /**
     * @brief Constructs a VolumeSlider with a given parameter name.
     * @param name The parameter name associated with volume control.
     */
    VolumeSlider(String name) : SynthSlider(name), point_y_(0), end_y_(1) {
      paintToImage(true);
      details_ = vital::Parameters::getDetails("volume");
    }

    /**
     * @brief Paints the custom arrow on the volume slider at the current value position.
     * @param g The Graphics context.
     */
    void paint(Graphics& g) override {
      float x = getPositionOfValue(getValue());

      Path arrow;
      arrow.startNewSubPath(x, point_y_);
      int arrow_height = end_y_ - point_y_;
      arrow.lineTo(x + arrow_height / 2.0f, end_y_);
      arrow.lineTo(x - arrow_height / 2.0f, end_y_);
      arrow.closeSubPath();
      g.setColour(findColour(Skin::kLinearSliderThumb, true));
      g.fillPath(arrow);
    }

    /**
     * @brief Sets the starting Y position of the arrow.
     * @param y The Y coordinate in pixels.
     */
    void setPointY(int y) { point_y_ = y; repaint(); }

    /**
     * @brief Sets the ending Y position of the arrow.
     * @param y The Y coordinate in pixels.
     */
    void setEndY(int y) { end_y_ = y; repaint(); }

    /**
     * @brief Gets the current ending Y position of the arrow.
     * @return The Y position in pixels.
     */
    int getEndY() { return end_y_; }

  private:
    vital::ValueDetails details_; ///< Parameter details for volume scaling and display.
    int point_y_;                 ///< The start Y coordinate for the arrow drawing.
    int end_y_;                   ///< The end Y coordinate for the arrow drawing.
};

VolumeSection::VolumeSection(String name) : SynthSection(name) {
  peak_meter_left_ = std::make_unique<PeakMeterViewer>(true);
  addOpenGlComponent(peak_meter_left_.get());
  peak_meter_right_ = std::make_unique<PeakMeterViewer>(false);
  addOpenGlComponent(peak_meter_right_.get());

  volume_ = std::make_unique<VolumeSlider>("volume");
  addSlider(volume_.get());
  volume_->setSliderStyle(Slider::LinearBar);
  volume_->setPopupPlacement(BubbleComponent::below);
}

VolumeSection::~VolumeSection() { }

int VolumeSection::getMeterHeight() {
  return getHeight() / 8;
}

int VolumeSection::getBuffer() {
  return getHeight() / 2 - getMeterHeight();
}

void VolumeSection::resized() {
  int meter_height = getMeterHeight();
  int volume_height = meter_height * 6.0f;
  int end_volume = meter_height * 3.5f;
  int padding = 1;
  int buffer = getBuffer();

  peak_meter_left_->setBounds(0, buffer, getWidth(), meter_height);
  peak_meter_right_->setBounds(0, peak_meter_left_->getBottom() + padding, getWidth(), meter_height);

  // Position the volume slider arrow according to the peak meter positions
  volume_->setPointY(peak_meter_right_->getBottom() + padding - buffer);
  volume_->setEndY(end_volume);
  volume_->setBounds(0, buffer, getWidth(), volume_height);

  SynthSection::resized();
}

void VolumeSection::paintBackground(Graphics& g) {
  SynthSection::paintKnobShadows(g);
  SynthSection::paintChildrenBackgrounds(g);

  int ticks_y = peak_meter_right_->getBottom() + getPadding();
  int tick_height = peak_meter_right_->getHeight() / 2;
  vital::ValueDetails details = vital::Parameters::getDetails("volume");

  g.setColour(findColour(Skin::kLightenScreen, true));
  // Draw vertical dB tick marks at intervals
  for (int decibel = -66; decibel <= 6; decibel += 6) {
    float offset = decibel - details.post_offset;
    float percent = offset * offset / (details.max - details.min);
    int x = percent * getWidth();
    g.drawRect(x, ticks_y, 1, tick_height);
  }
}
