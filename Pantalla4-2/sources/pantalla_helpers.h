#pragma once
#include "esphome.h"
#include "pantalla_config.h"
#include "pantalla_draw.h"
#include "pantalla_calendar_parser.h"
#include "pantalla_pagination.h"





// Helper functions for display drawing to keep YAML tidy and reusable.
// Usage: include this header with `esphome: includes: ["pantalla_helpers.h"]`

#include "pantalla_fonts.h"


// Check conditions: refresh enabled, people at home, time valid, time window
inline bool check_refresh_conditions() {
  ESP_LOGD("display", "check_refresh_conditions: refresh_enabled=%d, gente_en_casa=%d, force_refresh=%d",
           (int)id(refresh_enabled).state, (int)id(gente_en_casa).state, (int)id(force_refresh));

  if (!id(refresh_enabled).state && !id(force_refresh)) {
    ESP_LOGD("display", "check_refresh_conditions: skipped because refresh_enabled is false and no force_refresh");
    return false;
  }
  if (!id(gente_en_casa).state && !id(force_refresh)) {
    ESP_LOGD("display", "check_refresh_conditions: skipped because gente_en_casa is false and no force_refresh");
    return false;
  }

  auto now = id(esptime).now();
  bool now_valid = now.is_valid();
  if (!now_valid && !id(force_refresh)) {
    ESP_LOGD("display", "check_refresh_conditions: skipped because time invalid and no force_refresh");
    return false;
  }

  int current_hour = now_valid ? now.hour : 12;  // default noon
  int start_h = (int) id(refresh_start_hour).state;
  int end_h   = (int) id(refresh_end_hour).state;

  bool horario_ok;
  if (id(force_refresh)) {
    horario_ok = true;
  } else {
    if (start_h < end_h) {
      horario_ok = (current_hour >= start_h && current_hour < end_h);
    } else {
      horario_ok = (current_hour >= start_h || current_hour < end_h);
    }
  }
  if (!horario_ok) {
    ESP_LOGD("display", "check_refresh_conditions: skipped because outside time window");
    return false;
  }

  return true;
}

inline void draw_calendar(esphome::display::Display &it) {
  ESP_LOGD("display", "draw_calendar called");

  if (!check_refresh_conditions()) return;

  // Mark display active
  id(pantalla_activa) = true;
  ESP_LOGD("display", "pantalla_activa set to true");

  // Obtener configuración dinámica de UI
  auto now = id(esptime).now();
  bool now_valid = now.is_valid();
  auto* title_font = get_title_font();
  auto* date_font = get_date_font();
  int density = (int)id(ui_density_factor).state;
  int title_font_size = get_title_font_size();
  int date_font_size = get_date_font_size();

  // Limpiar pantalla
  it.fill(id(color_blanco));

  // Mostrar hora de último refresco (arriba-derecha, pequeña)
  char time_buf_small[6];
  if (now_valid) {
    snprintf(time_buf_small, sizeof(time_buf_small), "%02d:%02d", now.hour, now.minute);
    id(last_refresh) = std::string(time_buf_small);
  } else {
    snprintf(time_buf_small, sizeof(time_buf_small), "--:--");
    id(last_refresh) = std::string(time_buf_small);
  }
  draw_refresh_time(it, id(last_refresh));

  // ===== CALENDARIO =====
  int y = 0;
  std::string last_day = "";

  int device_num = get_device_number();

  // If device 1, require a minimum visible fraction before drawing (avoid partial events and orphan headers)
  if (device_num <= 1) {
    for (size_t i = 0; i < id(cal_title).size(); i++) {
      bool new_day = (id(cal_day)[i] != last_day);

      // Calculate height and visibility check (require DEVICE1_VISIBILITY_THRESHOLD to draw)
      int event_h = calc_event_height(i, title_font_size);
      int visible = SCREEN_H - y;
      int required = (int)ceilf(event_h * DEVICE1_VISIBILITY_THRESHOLD);
      int missing = event_h - visible;

      ESP_LOGI("pagination", "DRAW1 check i=%d y=%d event_h=%d visible=%d required=%d missing=%d", i, y, event_h, visible, required, missing);

      if (y + required > SCREEN_H) {
        ESP_LOGI("pagination", "DRAW1 skip i=%d (not enough visible for device 1)", i);
        break; // further events will be even less visible
      }

      // Fecha (solo una vez por día) - draw the header only if the event will be drawn
      if (new_day) {
        std::string day = id(cal_day)[i];
        int pos = day.find_last_of(' ');
        std::string day_num = day.substr(0, pos);
        std::string month   = day.substr(pos + 1);

        draw_date_header(it, DATE_COL_X, y, day_num, month, date_font, date_font_size, !last_day.empty());
        last_day = id(cal_day)[i];
      }

      // Draw event and get vertical advance
      int used = draw_event(it, y, i, title_font, title_font_size);
      y += used + density;
      ESP_LOGI("pagination", "DRAW1 painted i=%d -> y_next=%d used=%d density=%d", i, y, used, density);
    }
  } else {
    // For device > 1, compute start index by simulating previous devices
    ESP_LOGI("pagination", "draw_calendar called for device=%d sim_shrink=%d", device_num, (int)id(sim_shrink_pixels));

    // Compute start index by calling the canonical simulator
    int start_idx = calc_start_index(device_num, title_font_size, date_font_size, density);
    ESP_LOGD("diag", "draw_calendar: device=%d start_idx=%d", device_num, start_idx);



    // If starting mid-day, draw header for context only if the first event will actually be drawn
    if (start_idx > 0 && start_idx < (int)id(cal_title).size()) {
      int first_event_h = calc_event_height(start_idx, title_font_size);
      if ((0 + (int)(first_event_h * VISIBILITY_THRESHOLD)) <= SCREEN_H) {
        std::string day = id(cal_day)[start_idx];
        int pos = day.find_last_of(' ');
        std::string day_num = day.substr(0, pos);
        std::string month   = day.substr(pos + 1);
        // draw_separator = true for continuation to make it obvious
        draw_date_header(it, DATE_COL_X, y, day_num, month, date_font, date_font_size, true);
        last_day = id(cal_day)[start_idx];
      } else {
        ESP_LOGI("pagination", "SKIP context header at start_idx=%d because first event is not visible enough", start_idx);
      }
    }

    size_t i;
    for (i = start_idx; i < id(cal_title).size(); ++i) {
      bool new_day = (id(cal_day)[i] != last_day);

      // Visibility check (don't draw events that won't be mostly visible)
      int event_h = calc_event_height(i, title_font_size);
      int visible = SCREEN_H - y;
      int required = (int)ceilf(event_h * VISIBILITY_THRESHOLD);
      int missing = event_h - visible;

      ESP_LOGI("pagination", "DRAW check i=%d y=%d event_h=%d visible=%d required=%d missing=%d", i, y, event_h, visible, required, missing);

      if (y + (int)(event_h * VISIBILITY_THRESHOLD) > SCREEN_H) {
        // We'll skip drawing this event because it's not sufficiently visible
        ESP_LOGI("pagination", "DRAW skip i=%d (not enough visible)", i);
        break;
      }

      // Fecha (solo una vez por día) - draw the header only if the event will be drawn
      if (new_day) {
        std::string day = id(cal_day)[i];
        int pos = day.find_last_of(' ');
        std::string day_num = day.substr(0, pos);
        std::string month   = day.substr(pos + 1);

        draw_date_header(it, DATE_COL_X, y, day_num, month, date_font, date_font_size, !last_day.empty());
        last_day = id(cal_day)[i];
      }

      // Draw event and get vertical advance
      int used = draw_event(it, y, i, title_font, title_font_size);
      y += used + density;
    }

  }

  // Mostrar indicador de error de conexión en esquina inferior derecha
  draw_error_indicator(it, id(connection_error));
}
