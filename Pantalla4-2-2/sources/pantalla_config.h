#pragma once

// Screen dimensions and layout constants
constexpr int SCREEN_W = 400;
constexpr int SCREEN_H = 300;

// Columns / positions
constexpr int DATE_COL_X     = 5;
constexpr int TITLE_COL_X    = 70;
constexpr int RIGHT_MARGIN_X = 390; // approximate right margin for small elements
constexpr int TIME_DISPLAY_X = 400; // X position for timestamp (right aligned)

// Vertical spacing
constexpr int SPACING_WITH_LOCATION    = 34;
constexpr int SPACING_WITHOUT_LOCATION = 20;
constexpr int DAY_SEPARATOR_OFFSET     = -6;

// Text limits
constexpr int MAX_TITLE_CHARS = 38;
constexpr int MAX_EVENTS      = 32;  // increased to support multiple devices/pages
constexpr int MAX_DEVICES     = 4;   // maximum number of paginated devices supported
constexpr float VISIBILITY_THRESHOLD = 0.8f; // fraction of event height considered as 'painted' (80%)
constexpr float DEVICE1_VISIBILITY_THRESHOLD = 0.8f; // fraction of event height required to draw on device 1 (80%)

// Cache / TTL
constexpr int CACHE_TTL_SECONDS = 12 * 3600;

// Locale (Spanish short names)
static const char* const DIAS[] = {"dom.", "lun.", "mar.", "mié.", "jue.", "vie.", "sáb."};
static const char* const MESES[] = {"ene", "feb", "mar", "abr", "may", "jun", "jul", "ago", "sep", "oct", "nov", "dic"};

// Allowed characters (same glyph set used in fonts)
static const std::string ALLOWED_CHARS =
  "!\"#$%&'()*+,-./:;=?@°0123456789"
  "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz "
  "áéíóúÁÉÍÓÚñÑüÜ¿¡ÀÈÌÒÙàèìòùâêîôûÂÊÎÔÛäëïöÄËÏÖ";

// Structured event type shared across modules
struct CalendarEvent {
  std::string day;
  std::string title;
  std::string time;
  std::string location;
  std::string when;
};

inline std::vector<CalendarEvent>& cal_events_ref(); // defined in parser implementation
