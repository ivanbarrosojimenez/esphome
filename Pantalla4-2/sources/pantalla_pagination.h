#pragma once
#include "esphome.h"
#include "pantalla_config.h"
#include "pantalla_draw.h"
#include <cmath>
#include <vector>

// Device number storage via static local inside inline accessor functions (single instance)
inline int &device_number_ref() {
  static int v = 1; // default device 1
  return v;
}
inline void set_device_number(int n) { device_number_ref() = n; }
inline int get_device_number() { return device_number_ref(); }

// New overload: calculate the start index for a given device number by simulating the layout of prior devices using a structured events vector
inline int calc_start_index(int device_number, const std::vector<CalendarEvent> &events, int title_font_size, int date_font_size, int density) {
  ESP_LOGD("pagination", "calc_start_index called for device=%d sim_shrink_ui=%d sim_shrink_global=%d events=%d", device_number, (int)id(ui_sim_shrink_px).state, (int)id(sim_shrink_pixels), (int)events.size());
  if (device_number <= 1) return 0;

  int start_index = 0;
  const int total = (int)events.size();

  int last_painted_index = -1;
  for (int d = 1; d < device_number; ++d) {
    int y = 0;
    std::string last_day = "";

    std::vector<int> painted;
    for (int i = start_index; i < total; ++i) {
      if (events[i].day != last_day) {
        last_day = events[i].day;
      }

      bool next_same = (i + 1 < (int)events.size() && events[i].day == events[i + 1].day);
      int event_h = calc_event_height(events[i], next_same, title_font_size);
      int sim_shrink = (int) id(ui_sim_shrink_px).state;
      if (sim_shrink == 0) sim_shrink = (int) id(sim_shrink_pixels);
      // Treat sim_shrink as a reduction of available height (more conservative simulation)
      // NOTE: Do NOT apply sim_shrink when simulating the first device page (d == 1),
      // since the real draw uses the full SCREEN_H and we want simulator to match drawing.
      int threshold = SCREEN_H - (d == 1 ? 0 : sim_shrink);
      if (threshold < 0) threshold = 0;

      // Use per-device visibility threshold: device 1 uses DEVICE1_VISIBILITY_THRESHOLD, others use VISIBILITY_THRESHOLD
      float vis_frac = (d == 1) ? DEVICE1_VISIBILITY_THRESHOLD : VISIBILITY_THRESHOLD;
      int required = (int)ceilf(event_h * vis_frac);

      ESP_LOGD("pagination", "SIM check d=%d i=%d y=%d event_h=%d required=%d sim_shrink=%d threshold=%d", d, i, y, event_h, required, sim_shrink, threshold);

      // Use the exact same check as drawing: if painting the required fraction would exceed the (simulated) threshold, stop
      if (y + required > threshold) {
        ESP_LOGD("pagination", "SIM stop d=%d i=%d y=%d required=%d threshold=%d start_index=%d", d, i, y, required, threshold, start_index);
        break;
      }

      // Event considered painted — advance (advance by full event height + density to mirror draw_event)
      y += event_h + density;
      painted.push_back(i);
      start_index = i + 1;
      last_painted_index = painted.back();
      ESP_LOGD("pagination", "SIM painted d=%d i=%d -> next start_index=%d y=%d (used=%d density=%d)", d, i, start_index, y, event_h, density);

      if (start_index >= total) {
        ESP_LOGD("pagination", "SIM device=%d painted indices: first=%d last=%d count=%d -> consumed all events", d, painted.front(), painted.back(), (int)painted.size());
        // Ensure start_index is after last painted
        start_index = std::max(start_index, last_painted_index + 1);
        return start_index; // consumed all events
      }
    }
    ESP_LOGD("pagination", "SIM device=%d painted indices: count=%d last_start_index=%d last_painted_index=%d", d, (int)painted.size(), start_index, last_painted_index);

    if (start_index == 0) break; // no progress, avoid infinite loop
  }

  // Defensive: ensure start_index is at least one past the last painted index
  if (last_painted_index >= 0) {
    int corrected = last_painted_index + 1;
    if (corrected > start_index) {
      ESP_LOGD("pagination", "SIM correcting start_index from %d to %d based on last_painted_index", start_index, corrected);
      start_index = corrected;
    }
  }

  // Post-check: ensure the target device will paint at least one event; if not, step start_index back (up to a few events) to avoid skipping many events
  {
    int test_start = start_index;
    int attempts = 0;
    while (test_start > 0 && attempts < 6) {
      int y_test = 0;
      int painted_test = 0;
      for (int i = test_start; i < total; ++i) {
        bool next_same = (i + 1 < (int)events.size() && events[i].day == events[i + 1].day);
        int event_h = calc_event_height(events[i], next_same, title_font_size);
        float vis_frac = (device_number == 1) ? DEVICE1_VISIBILITY_THRESHOLD : VISIBILITY_THRESHOLD;
        int required = (int)ceilf(event_h * vis_frac);
        if (y_test + required > SCREEN_H) break;
        painted_test++;
        y_test += event_h + density;
        if (painted_test > 0) break; // we only need one
      }
      if (painted_test > 0) break;
      test_start--; attempts++;
    }
    if (test_start != start_index) {
      ESP_LOGD("pagination", "Post-adjust start_index from %d to %d (ensured next device paints) ", start_index, test_start);
      start_index = test_start;
    }
  }

  ESP_LOGD("pagination", "calc_start_index result for device=%d -> start_index=%d", device_number, start_index);
  return start_index;
}

// Backwards-compatible wrapper: use structured events vector
inline int calc_start_index(int device_number, int title_font_size, int date_font_size, int density) {
  return calc_start_index(device_number, cal_events_ref(), title_font_size, date_font_size, density);
}