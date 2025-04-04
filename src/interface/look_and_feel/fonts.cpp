#include "fonts.h"

Fonts::Fonts() :
    proportional_regular_(Typeface::createSystemTypefaceFor(
        BinaryData::LatoRegular_ttf, BinaryData::LatoRegular_ttfSize)),
    proportional_light_(Typeface::createSystemTypefaceFor(
        BinaryData::LatoLight_ttf, BinaryData::LatoLight_ttfSize)),
    proportional_title_(Typeface::createSystemTypefaceFor(
        BinaryData::MontserratLight_otf, BinaryData::MontserratLight_otfSize)),
    proportional_title_regular_(Typeface::createSystemTypefaceFor(
        BinaryData::MontserratRegular_ttf, BinaryData::MontserratRegular_ttfSize)),
    monospace_(Typeface::createSystemTypefaceFor(
        BinaryData::DroidSansMono_ttf, BinaryData::DroidSansMono_ttfSize)) {

// Preloads the glyph positions to ensure fonts are fully initialized.
  Array<int> glyphs;
  Array<float> x_offsets;
  proportional_regular_.getGlyphPositions("test", glyphs, x_offsets);
  proportional_light_.getGlyphPositions("test", glyphs, x_offsets);
  proportional_title_.getGlyphPositions("test", glyphs, x_offsets);
  monospace_.getGlyphPositions("test", glyphs, x_offsets);
}
