#pragma once
#include "esphome.h"
#include "pantalla_config.h"

// Definition of cal_events_ref (structured vector storage)
inline std::vector<CalendarEvent>& cal_events_ref() {
  static std::vector<CalendarEvent> v;
  return v;
}

// Trim spaces
inline void trim(std::string &s) {
  s.erase(s.begin(), std::find_if(s.begin(), s.end(), [](unsigned char ch) { return !std::isspace(ch); }));
  s.erase(std::find_if(s.rbegin(), s.rend(), [](unsigned char ch) { return !std::isspace(ch); }).base(), s.end());
}

// Normalize using ALLOWED_CHARS
inline void normalize(std::string &s) {
  for (char &c : s) {
    if (ALLOWED_CHARS.find(c) == std::string::npos) {
      c = ' ';
    }
  }
}

// Truncate
inline void truncate_str(std::string &s, size_t max_len) {
  if (s.size() > max_len) s = s.substr(0, max_len);
}

// FNV-1a 64-bit -> hex string
inline std::string fnv1a_hash_hex(const std::string &data) {
  uint64_t hash = 14695981039346656037ULL;
  for (char c : data) {
    hash ^= (uint8_t)c;
    hash *= 1099511628211ULL;
  }
  char buf[17];
  snprintf(buf, sizeof(buf), "%016llx", (unsigned long long)hash);
  return std::string(buf);
}

// Parse ISO date (YYYY-MM-DD...) into y,m,d,hh,mm. Returns false if too short
inline bool parse_iso_datetime(const std::string &iso, int &y, int &m, int &d, int &hh, int &mm) {
  if (iso.size() < 10) return false;
  // Try common formats with sscanf to avoid exceptions
  hh = 0; mm = 0;
  int matched = 0;
  // Try full datetime with 'T'
  matched = sscanf(iso.c_str(), "%4d-%2d-%2dT%2d:%2d", &y, &m, &d, &hh, &mm);
  if (matched >= 3) {
    if (matched < 5) { hh = 0; mm = 0; }
    return true;
  }
  // Try datetime with space
  matched = sscanf(iso.c_str(), "%4d-%2d-%2d %2d:%2d", &y, &m, &d, &hh, &mm);
  if (matched >= 3) {
    if (matched < 5) { hh = 0; mm = 0; }
    return true;
  }
  // Try date only
  matched = sscanf(iso.c_str(), "%4d-%2d-%2d", &y, &m, &d);
  if (matched == 3) {
    hh = 0; mm = 0;
    return true;
  }
  return false;
}

// Original inline processing logic replaced by a cleaner, testable implementation using parse_events_from_json().
// See the simplified `procesar_calendario_impl()` below which is smaller and delegates parsing to the helper.


// Parse JSON events into a structured vector of CalendarEvent
inline int parse_events_from_json(const std::string &json_str, std::vector<CalendarEvent> &out, time_t now_ts) {
  out.clear();
  if (json_str.empty()) return 0;

  #pragma GCC diagnostic push
  #pragma GCC diagnostic ignored "-Wdeprecated-declarations"
  StaticJsonDocument<16384> doc;
  #pragma GCC diagnostic pop
  DeserializationError err = deserializeJson(doc, json_str);
  if (err) {
    ESP_LOGE("calendar", "Error JSON: %s", err.c_str());
    return 0;
  }

  JsonArray arr = doc.as<JsonArray>();
  int shown = 0;

  for (JsonObject ev : arr) {
    if (shown >= MAX_EVENTS) break;

    std::string start = ev["start"] | "";
    std::string end   = ev["end"]   | "";
    std::string title = ev["summary"] | "";
    std::string loc   = ev["location"] | "";

    int y,m,d,hh,mm;
    if (!parse_iso_datetime(start, y, m, d, hh, mm)) continue;

    tm timeinfo = {};
    timeinfo.tm_year = y - 1900;
    timeinfo.tm_mon  = m - 1;
    timeinfo.tm_mday = d;
    timeinfo.tm_hour = hh;
    timeinfo.tm_min  = mm;
    timeinfo.tm_sec  = 0;
    timeinfo.tm_isdst = -1;

    mktime(&timeinfo); // compute tm_wday

    // Normalize to midday to avoid DST edge issues
    tm today_tm = {};
    today_tm.tm_year = (int)localtime(&now_ts)->tm_year;
    today_tm.tm_mon  = (int)localtime(&now_ts)->tm_mon;
    today_tm.tm_mday = (int)localtime(&now_ts)->tm_mday;
    today_tm.tm_hour = 12;
    today_tm.tm_min  = 0;
    today_tm.tm_sec  = 0;
    time_t today_ts = mktime(&today_tm);

    tm ev_tm = timeinfo;
    ev_tm.tm_hour = 12; ev_tm.tm_min = 0; ev_tm.tm_sec = 0;
    time_t event_ts = mktime(&ev_tm);

    int days = (event_ts - today_ts) / 86400;

    char when_buf[20];
    if (days < 0) continue; // skip past events
    else if (days == 0) snprintf(when_buf, sizeof(when_buf), "(hoy)");
    else if (days == 1) snprintf(when_buf, sizeof(when_buf), "(mañana)");
    else if (days < 30) snprintf(when_buf, sizeof(when_buf), "(en %d días)", days);
    else snprintf(when_buf, sizeof(when_buf), "(+1 mes)");

    char day_buf[40];
    snprintf(day_buf, sizeof(day_buf), "%s %d %s",
             DIAS[timeinfo.tm_wday], timeinfo.tm_mday, MESES[timeinfo.tm_mon]);

    char time_buf[40];
    if (start.size() >= 16 && end.size() >= 16) {
      snprintf(time_buf, sizeof(time_buf), "%s-%s", start.substr(11,5).c_str(), end.substr(11,5).c_str());
    } else {
      snprintf(time_buf, sizeof(time_buf), "Todo el día");
    }

    std::string day_str(day_buf); trim(day_str); normalize(day_str);

    trim(title); normalize(title);

    std::string time_str(time_buf); trim(time_str); normalize(time_str);

    trim(loc); normalize(loc); truncate_str(loc, 50);

    std::string when_str(when_buf); trim(when_str); normalize(when_str);

    CalendarEvent ce{day_str, title, time_str, loc, when_str};
    out.push_back(ce);

    ESP_LOGD("calendar", "Evento añadido: %s", title.c_str());
    ESP_LOGD("calendar", "Location añadido: %s", loc.c_str());
    shown++;
  }

  return shown;
}

// Update: procesar_calendario_impl simplified to call the parse helper
inline void procesar_calendario_impl() {
  id(cal_day).clear();
  id(cal_title).clear();
  id(cal_time).clear();
  id(cal_location).clear();
  id(cal_when).clear();
  cal_events_ref().clear();

  id(connection_error) = "";
  id(using_cache) = false;

  if (!WiFi.isConnected()) {
    id(connection_error) = "Sin WiFi";
    ESP_LOGW("calendar", "WiFi not connected");
  }

  const std::string &json_sensor = id(calendario_eventos_raw).state;
  std::string json_str;
  if (!json_sensor.empty()) {
    json_str = json_sensor;
    id(calendario_json) = json_sensor;
    if (id(connection_error).empty() || id(connection_error) == "Sin WiFi") id(connection_error) = "";
  } else if (!id(calendario_json).empty()) {
    json_str = id(calendario_json);
    id(using_cache) = true;
    if (id(connection_error).empty()) id(connection_error) = "Sin HA";
    ESP_LOGI("calendar", "Usando JSON cacheado");
  } else {
    if (id(connection_error).empty()) id(connection_error) = "Sin datos";
    ESP_LOGW("calendar", "JSON vacío");
    id(cal_changed) = false;
    return;
  }

  ESP_LOGI("calendar", "procesar_calendario: sensor_len=%d cache_len=%d last_hash=%s cache_ts=%d",
           (int)json_sensor.size(), (int)id(calendario_json).size(), id(last_calendar_hash).c_str(), id(calendario_cache_ts));

  auto now = id(esptime).now();
  if (!now.is_valid()) {
    ESP_LOGW("calendar", "Hora no válida");
    id(cal_changed) = false;
    return;
  }

  tm now_tm = {};
  now_tm.tm_year = now.year - 1900;
  now_tm.tm_mon  = now.month - 1;
  now_tm.tm_mday = now.day_of_month;
  now_tm.tm_hour = now.hour;
  now_tm.tm_min  = now.minute;
  now_tm.tm_sec  = now.second;
  time_t now_ts = mktime(&now_tm);

  if (!json_sensor.empty()) {
    id(calendario_cache_ts) = (int)now_ts;
  } else if (id(calendario_cache_ts) != 0) {
    if ((now_ts - (time_t)id(calendario_cache_ts)) > CACHE_TTL_SECONDS) {
      ESP_LOGW("calendar", "Cached JSON stale (%ld s), will request refresh", (long)(now_ts - (time_t)id(calendario_cache_ts)));
      id(force_refresh) = true;
    }
  }

  // Parse events using helper
  int shown = parse_events_from_json(json_str, cal_events_ref(), now_ts);

  // Keep backwards-compatible parallel arrays in sync for existing draw code
  for (auto &ev : cal_events_ref()) {
    id(cal_day).push_back(ev.day);
    id(cal_title).push_back(ev.title);
    id(cal_time).push_back(ev.time);
    id(cal_location).push_back(ev.location);
    id(cal_when).push_back(ev.when);
  }

  // Update last refresh timestamp
  char ts_buf[32];
  snprintf(ts_buf, sizeof(ts_buf), "%04d-%02d-%02d %02d:%02d:%02d",
           now.year, now.month, now.day_of_month, now.hour, now.minute, now.second);
  id(last_refresh) = std::string(ts_buf);

  ESP_LOGI("calendar", "Eventos procesados: %d", shown);

  // Compute processed data hash from structured events
  std::string processed_data;
  for (size_t i = 0; i < cal_events_ref().size(); ++i) {
    auto &e = cal_events_ref()[i];
    processed_data += e.title + "|" + e.location + "|" + e.time + "|" + e.when + "|" + e.day + "\n";
  }
  std::string proc_hash_str = fnv1a_hash_hex(processed_data);

  if (proc_hash_str != id(last_processed_hash) || id(force_refresh)) {
    id(cal_changed) = true;
    id(last_processed_hash) = proc_hash_str;
    ESP_LOGI("calendar", "Processed data changed (hash %s), will paint", proc_hash_str.c_str());
  } else {
    id(cal_changed) = false;
    ESP_LOGI("calendar", "No changes in processed data (hash %s), skipping paint", proc_hash_str.c_str());
  }
}
