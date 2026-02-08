#pragma once
#include "esphome.h"
#include "pantalla_config.h"
#include "pantalla_fonts.h"

// Split text into up to two lines, breaking at last space before max_chars
inline void split_two_lines(const std::string &text, std::string &l1, std::string &l2, int max_chars) {
  if ((int)text.size() <= max_chars) {
    l1 = text;
    l2.clear();
    return;
  }

  int cut = max_chars;
  for (int i = max_chars; i > 0; i--) {
    if (text[i] == ' ') {
      cut = i;
      break;
    }
  }

  l1 = text.substr(0, cut);
  l2 = text.substr(cut + 1);

  if ((int)l2.size() > max_chars) {
    l2 = l2.substr(0, max_chars - 3) + "...";
  }
}

// Draw the small location (map pin) icon. Coordinates are the top-left reference of the icon area.
inline void draw_location_icon(esphome::display::Display &it, int icon_x, int icon_top, esphome::Color color) {
  // Draw the rounded top arc using short segments (thin lines suitable for small icon)
  it.line(icon_x + 1, icon_top + 7, icon_x + 1, icon_top + 5, color);
  it.line(icon_x + 1, icon_top + 5, icon_x + 3, icon_top + 2, color);
  it.line(icon_x + 3, icon_top + 2, icon_x + 6, icon_top + 1, color);

  it.line(icon_x + 6, icon_top + 1, icon_x + 9, icon_top + 2, color);
  it.line(icon_x + 9, icon_top + 2, icon_x + 11, icon_top + 5, color);
  it.line(icon_x + 11, icon_top + 5, icon_x + 11, icon_top + 7, color);

  // Sides converging to the tip
  it.line(icon_x + 1, icon_top + 7, icon_x + 2, icon_top + 9, color);
  it.line(icon_x + 2, icon_top + 9, icon_x + 4, icon_top + 11, color);
  it.line(icon_x + 4, icon_top + 11, icon_x + 6, icon_top + 13, color);

  it.line(icon_x + 11, icon_top + 7, icon_x + 10, icon_top + 9, color);
  it.line(icon_x + 10, icon_top + 9, icon_x + 8, icon_top + 11, color);
  it.line(icon_x + 8, icon_top + 11, icon_x + 6, icon_top + 13, color);

  // Inner small circle
  it.circle(icon_x + 6, icon_top + 5, 2, color);

  // Base ellipse simulated with thin lines
  it.line(icon_x + 3, icon_top + 14, icon_x + 9, icon_top + 14, color);
  it.line(icon_x + 1, icon_top + 15, icon_x + 11, icon_top + 15, color);
  it.line(icon_x + 3, icon_top + 16, icon_x + 9, icon_top + 16, color);
}

inline void draw_refresh_time(esphome::display::Display &it, const std::string &time_str) {
  auto* tf = get_time_font();
  int tf_sz = get_time_font_size();
  it.print(TIME_DISPLAY_X, -1, tf, id(color_negro), TextAlign::TOP_RIGHT, time_str.c_str());
}

inline void draw_error_indicator(esphome::display::Display &it, const std::string &error) {
  if (error.empty()) return;
  std::string error_msg = "(" + error + ")";
  auto* tf = get_time_font();
  it.print(SCREEN_W - 5, SCREEN_H - 5, tf, id(color_dark_gray), TextAlign::BOTTOM_RIGHT, error_msg.c_str());
}

// Draw the date header (day number + month) and optional separator above it
inline void draw_date_header(esphome::display::Display &it, int x, int y, const std::string &day_num, const std::string &month, esphome::display::BaseFont* date_font, int date_font_size, bool draw_separator) {
  it.print(x, y, date_font, id(color_negro), day_num.c_str());
  it.print(x, y + date_font_size, date_font, id(color_negro), month.c_str());
  if (draw_separator) {
    it.line(0, y + DAY_SEPARATOR_OFFSET, SCREEN_W - 1, y + DAY_SEPARATOR_OFFSET, id(color_negro));
  }
}

// Calculate event height without drawing (mirrors draw_event's returned height)
inline int calc_event_height(size_t i, int title_font_size) {
  std::string t1, t2;
  split_two_lines(id(cal_title)[i], t1, t2, MAX_TITLE_CHARS);
  int title_height_single = title_font_size + 2;
  int title_height = t2.empty() ? title_height_single : (title_height_single * 2);
  int spacing = id(cal_location)[i].empty() ? SPACING_WITHOUT_LOCATION : SPACING_WITH_LOCATION;
  // Reduce spacing by 4 px when the next event is on the same day (same logic as draw_event)
  if (i + 1 < id(cal_day).size() && id(cal_day)[i] == id(cal_day)[i + 1]) {
    spacing = spacing - 4;
    if (spacing < 0) spacing = 0;
  }
  return title_height + spacing;
}

// Draw a single event. Returns the total vertical space used (title height + spacing)
inline int draw_event(esphome::display::Display &it, int y, size_t i, esphome::display::BaseFont* title_font, int title_font_size) {
  // Split title into up to two lines
  std::string t1, t2;
  split_two_lines(id(cal_title)[i], t1, t2, MAX_TITLE_CHARS);

  it.print(TITLE_COL_X, y, title_font, id(color_negro), t1.c_str());

  int title_height_single = title_font_size + 2;
  int title_height = t2.empty() ? title_height_single : (title_height_single * 2);
  if (!t2.empty()) {
    it.print(TITLE_COL_X, y + title_height_single, title_font, id(color_negro), t2.c_str());
  }

  int y_line = y + title_height;

  // Hora
  // Time and "when" use dedicated selector
  auto* time_font = get_time_font();
  int time_font_size = get_time_font_size();
  it.print(TITLE_COL_X, y_line, time_font, id(color_negro), id(cal_time)[i].c_str());

  // Cuándo + índice del evento (índice mostrado a la derecha, empezando en 1)
  const int INDEX_OFFSET_PX = 20; // espacio reservado para el número (dos dígitos)
  std::string when_str = id(cal_when)[i];
  it.print(RIGHT_MARGIN_X - INDEX_OFFSET_PX, y_line, time_font, id(color_negro), TextAlign::TOP_RIGHT, when_str.c_str());
  // Índice en gris oscuro a la derecha
  std::string idx = std::to_string((int)i + 1);
  it.print(RIGHT_MARGIN_X, y_line, time_font, id(color_dark_gray), TextAlign::TOP_RIGHT, idx.c_str());

  // Localización
  if (!id(cal_location)[i].empty()) {
    int icon_x = TITLE_COL_X - 14;
    int icon_top = y + title_height + 10;
    draw_location_icon(it, icon_x, icon_top, id(color_negro));
    auto* loc_font = get_location_font();
    int loc_font_size = get_location_font_size();
    it.print(icon_x + 14, y + title_height + 14, loc_font, id(color_dark_gray), id(cal_location)[i].c_str());
  }

  int spacing = id(cal_location)[i].empty() ? SPACING_WITHOUT_LOCATION : SPACING_WITH_LOCATION;
  // Reduce spacing by 4 px when the following event is on the same day
  if (i + 1 < id(cal_day).size() && id(cal_day)[i] == id(cal_day)[i + 1]) {
    spacing = spacing - 4;
    if (spacing < 0) spacing = 0;
  }

  return title_height + spacing;
}