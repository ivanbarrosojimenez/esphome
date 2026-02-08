#pragma once
#include "esphome.h"

// Font resolver helpers
// Helper: parse strings like "16" or "16 bold" -> (size, bold)
inline std::pair<int,bool> parse_font_option(const std::string &opt) {
  if (opt.empty()) return {16, false};
  // find digits at start
  int size = 0;
  for (char c : opt) {
    if (c >= '0' && c <= '9') {
      size = size * 10 + (c - '0');
    } else break;
  }
  bool bold = (opt.find("bold") != std::string::npos);
  if (size == 0) size = 16;
  if (size > 20) size = 20;
  return {size, bold};
}

// Unified font selector: UI tamaño fuente (default 16)
inline esphome::display::BaseFont* get_title_font() {
  auto [size, bold] = parse_font_option(id(ui_font_size).state);
  // map to available font ids
  if (!bold) {
    if (size == 12) return static_cast<esphome::display::BaseFont*>(&id(fuente_titulo_12));
    if (size == 13) return static_cast<esphome::display::BaseFont*>(&id(fuente_titulo_13));
    if (size == 14) return static_cast<esphome::display::BaseFont*>(&id(fuente_titulo_14));
    if (size == 15) return static_cast<esphome::display::BaseFont*>(&id(fuente_titulo_15));
    if (size == 16) return static_cast<esphome::display::BaseFont*>(&id(fuente_titulo_16));
    if (size == 17) return static_cast<esphome::display::BaseFont*>(&id(fuente_titulo_17));
    if (size == 18) return static_cast<esphome::display::BaseFont*>(&id(fuente_titulo_18));
    if (size == 19) return static_cast<esphome::display::BaseFont*>(&id(fuente_titulo_19));
    // default/fallback to 20
    return static_cast<esphome::display::BaseFont*>(&id(fuente_titulo_20));
  } else {
    if (size == 12) return static_cast<esphome::display::BaseFont*>(&id(fuente_titulo_bold_12));
    if (size == 13) return static_cast<esphome::display::BaseFont*>(&id(fuente_titulo_bold_13));
    if (size == 14) return static_cast<esphome::display::BaseFont*>(&id(fuente_titulo_bold_14));
    if (size == 15) return static_cast<esphome::display::BaseFont*>(&id(fuente_titulo_bold_15));
    if (size == 16) return static_cast<esphome::display::BaseFont*>(&id(fuente_titulo_bold_16));
    if (size == 17) return static_cast<esphome::display::BaseFont*>(&id(fuente_titulo_bold_17));
    if (size == 18) return static_cast<esphome::display::BaseFont*>(&id(fuente_titulo_bold_18));
    if (size == 19) return static_cast<esphome::display::BaseFont*>(&id(fuente_titulo_bold_19));
    return static_cast<esphome::display::BaseFont*>(&id(fuente_titulo_bold_20));
  }
}

inline int get_title_font_size() {
  auto [size, bold] = parse_font_option(id(ui_font_size).state);
  (void)bold;
  return size;
}

// Date fonts use same selector; map larger sizes to nearest available date font
inline esphome::display::BaseFont* get_date_font() {
  auto [size, bold] = parse_font_option(id(ui_date_font_size).state);
  // Handle size 12 specially: use small date font unless bold requested
  if (size == 12) {
    if (bold) return static_cast<esphome::display::BaseFont*>(&id(fuente_titulo_bold_12));
    return static_cast<esphome::display::BaseFont*>(&id(fuente_fecha_12));
  }
  if (!bold) {
    if (size == 13) return static_cast<esphome::display::BaseFont*>(&id(fuente_titulo_13));
    if (size == 14) return static_cast<esphome::display::BaseFont*>(&id(fuente_titulo_14));
    if (size == 15) return static_cast<esphome::display::BaseFont*>(&id(fuente_titulo_15));
    if (size == 16) return static_cast<esphome::display::BaseFont*>(&id(fuente_titulo_16));
    if (size == 17) return static_cast<esphome::display::BaseFont*>(&id(fuente_titulo_17));
    if (size == 18) return static_cast<esphome::display::BaseFont*>(&id(fuente_titulo_18));
    if (size == 19) return static_cast<esphome::display::BaseFont*>(&id(fuente_titulo_19));
    return static_cast<esphome::display::BaseFont*>(&id(fuente_titulo_20));
  } else {
    if (size == 13) return static_cast<esphome::display::BaseFont*>(&id(fuente_titulo_bold_13));
    if (size == 14) return static_cast<esphome::display::BaseFont*>(&id(fuente_titulo_bold_14));
    if (size == 15) return static_cast<esphome::display::BaseFont*>(&id(fuente_titulo_bold_15));
    if (size == 16) return static_cast<esphome::display::BaseFont*>(&id(fuente_titulo_bold_16));
    if (size == 17) return static_cast<esphome::display::BaseFont*>(&id(fuente_titulo_bold_17));
    if (size == 18) return static_cast<esphome::display::BaseFont*>(&id(fuente_titulo_bold_18));
    if (size == 19) return static_cast<esphome::display::BaseFont*>(&id(fuente_titulo_bold_19));
    return static_cast<esphome::display::BaseFont*>(&id(fuente_titulo_bold_20));
  }
}

inline int get_date_font_size() {
  auto [size, bold] = parse_font_option(id(ui_date_font_size).state);
  (void)bold;
  return size;
}

// Time font selectors: used for the event time and the "when" (e.g., "en 2 días")
inline esphome::display::BaseFont* get_time_font() {
  auto [size, bold] = parse_font_option(id(ui_time_font_size).state);
  // If size 12 and bold requested, use bold 12; otherwise use small date 12
  if (size == 12) {
    if (bold) return static_cast<esphome::display::BaseFont*>(&id(fuente_titulo_bold_12));
    return static_cast<esphome::display::BaseFont*>(&id(fuente_fecha_12));
  }
  if (!bold) {
    if (size == 13) return static_cast<esphome::display::BaseFont*>(&id(fuente_titulo_13));
    if (size == 14) return static_cast<esphome::display::BaseFont*>(&id(fuente_titulo_14));
    if (size == 15) return static_cast<esphome::display::BaseFont*>(&id(fuente_titulo_15));
    if (size == 16) return static_cast<esphome::display::BaseFont*>(&id(fuente_titulo_16));
    if (size == 17) return static_cast<esphome::display::BaseFont*>(&id(fuente_titulo_17));
    if (size == 18) return static_cast<esphome::display::BaseFont*>(&id(fuente_titulo_18));
    if (size == 19) return static_cast<esphome::display::BaseFont*>(&id(fuente_titulo_19));
    return static_cast<esphome::display::BaseFont*>(&id(fuente_titulo_20));
  } else {
    if (size == 13) return static_cast<esphome::display::BaseFont*>(&id(fuente_titulo_bold_13));
    if (size == 14) return static_cast<esphome::display::BaseFont*>(&id(fuente_titulo_bold_14));
    if (size == 15) return static_cast<esphome::display::BaseFont*>(&id(fuente_titulo_bold_15));
    if (size == 16) return static_cast<esphome::display::BaseFont*>(&id(fuente_titulo_bold_16));
    if (size == 17) return static_cast<esphome::display::BaseFont*>(&id(fuente_titulo_bold_17));
    if (size == 18) return static_cast<esphome::display::BaseFont*>(&id(fuente_titulo_bold_18));
    if (size == 19) return static_cast<esphome::display::BaseFont*>(&id(fuente_titulo_bold_19));
    return static_cast<esphome::display::BaseFont*>(&id(fuente_titulo_bold_20));
  }
}

inline int get_time_font_size() {
  auto [size, bold] = parse_font_option(id(ui_time_font_size).state);
  (void)bold;
  return size;
}

// Location font selectors: used to render the event location text
inline esphome::display::BaseFont* get_location_font() {
  auto [size, bold] = parse_font_option(id(ui_location_font_size).state);
  if (size == 12) {
    if (bold) return static_cast<esphome::display::BaseFont*>(&id(fuente_titulo_bold_12));
    return static_cast<esphome::display::BaseFont*>(&id(fuente_titulo_12));
  }
  if (!bold) {
    if (size == 13) return static_cast<esphome::display::BaseFont*>(&id(fuente_titulo_13));
    if (size == 14) return static_cast<esphome::display::BaseFont*>(&id(fuente_titulo_14));
    if (size == 15) return static_cast<esphome::display::BaseFont*>(&id(fuente_titulo_15));
    if (size == 16) return static_cast<esphome::display::BaseFont*>(&id(fuente_titulo_16));
    if (size == 17) return static_cast<esphome::display::BaseFont*>(&id(fuente_titulo_17));
    if (size == 18) return static_cast<esphome::display::BaseFont*>(&id(fuente_titulo_18));
    if (size == 19) return static_cast<esphome::display::BaseFont*>(&id(fuente_titulo_19));
    return static_cast<esphome::display::BaseFont*>(&id(fuente_titulo_20));
  } else {
    if (size == 13) return static_cast<esphome::display::BaseFont*>(&id(fuente_titulo_bold_13));
    if (size == 14) return static_cast<esphome::display::BaseFont*>(&id(fuente_titulo_bold_14));
    if (size == 15) return static_cast<esphome::display::BaseFont*>(&id(fuente_titulo_bold_15));
    if (size == 16) return static_cast<esphome::display::BaseFont*>(&id(fuente_titulo_bold_16));
    if (size == 17) return static_cast<esphome::display::BaseFont*>(&id(fuente_titulo_bold_17));
    if (size == 18) return static_cast<esphome::display::BaseFont*>(&id(fuente_titulo_bold_18));
    if (size == 19) return static_cast<esphome::display::BaseFont*>(&id(fuente_titulo_bold_19));
    return static_cast<esphome::display::BaseFont*>(&id(fuente_titulo_bold_20));
  }
}

inline int get_location_font_size() {
  auto [size, bold] = parse_font_option(id(ui_location_font_size).state);
  (void)bold;
  return size;
} 
