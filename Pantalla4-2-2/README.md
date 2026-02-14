# Plan de Refactorización — Pantalla Calendario e-Paper

## Estado actual

| Archivo | Líneas | Rol |
|---|---|---|
| `pantalla-frigate.yml` | ~853 | Config ESPHome + ~290 líneas de C++ inline en lambdas |
| `sources/pantalla_helpers.h` | ~260 | Funciones de dibujo (1 función monolítica de ~170 líneas) |
| `custom_components/waveshare_epaper/` | — | Fork local del driver e-paper 4.2" V2 |
| `custom_components/ssd1680/` | — | Carpeta vacía (sin uso) |

---

## Problemas identificados

### 🔴 Críticos (alto impacto)

| # | Problema | Ubicación |
|---|---|---|
| 1 | **~290 líneas de C++ incrustadas en YAML** — El script `procesar_calendario` contiene parsing JSON, cálculo de hashes, lógica de fechas, locale español, etc. todo en un lambda inline | `pantalla-frigate.yml` (script `procesar_calendario`) |
| 2 | **`draw_calendar` es una función monolítica de ~170 líneas** — Gestiona condiciones de refresco, selección de fuentes, limpieza de pantalla, iteración de eventos, dibujo de iconos, espaciado, errores | `sources/pantalla_helpers.h` |
| 3 | **Cadena de glifos copiada 12 veces** — Cada definición de fuente repite la misma cadena de glifos idéntica | `pantalla-frigate.yml` (sección `font:`) |
| 4 | **`allowed_chars` duplica la cadena de glifos** — Si se cambian los glifos en las fuentes, hay que actualizar también esta variable por separado | `pantalla-frigate.yml` (lambda `procesar_calendario`) |

### 🟡 Medios (mantenibilidad)

| # | Problema | Ubicación |
|---|---|---|
| 5 | **IDs de fuente no coinciden con su tamaño** — `fuente_28` es tamaño 20, `fuente_14` es tamaño 16 | `pantalla-frigate.yml` |
| 6 | **Hash FNV-1a duplicado** — El mismo algoritmo de hash se computa dos veces con código copy-paste | Lambda `procesar_calendario` |
| 7 | **Icono de localización dibujado con ~20 llamadas a `line`/`circle`** — Código de dibujo mezclado con la lógica del calendario | `src/pantalla_helpers.h` |
| 8 | **Funciones de fuente con patrón duplicado** — `get_title_font`/`get_title_font_size` y `get_date_font`/`get_date_font_size` son estructuras casi idénticas | `src/pantalla_helpers.h` |
| 9 | **3 ramas idénticas en lógica de spacing** — El `if/else if/else` produce exactamente el mismo resultado en las tres ramas | `src/pantalla_helpers.h` |
| 10 | **Números mágicos para dimensiones y layout** — `400`, `399`, `395`, `295`, `5`, `70`, `380`, `-6`, `38`, `34`, `20` repartidos por el código | `src/pantalla_helpers.h` |

### 🟢 Menores (limpieza)

| # | Problema | Ubicación |
|---|---|---|
| 11 | **Max eventos hardcodeado como `8`** | Lambda `procesar_calendario` |
| 12 | **Cadenas de locale español hardcodeadas** — `dias[]`, `meses[]` | Lambda `procesar_calendario` |
| 13 | **Parsing manual de fechas ISO-8601** — `stoi()` + `substr()` con offsets fijos, frágil | Lambda `procesar_calendario` |
| 14 | **Variable `now_tm` declarada dos veces** (shadowing) | Lambda `procesar_calendario` |
| 15 | **Comentarios de default incorrectos** — `get_title_font` dice "default 20" pero devuelve `fuente_28` | `src/pantalla_helpers.h` |
| 16 | **`fuente_8_2` (Roboto 12) aparentemente sin uso** | `pantalla-frigate.yml` |
| 17 | **Carpeta `custom_components/ssd1680/` vacía** | Filesystem |
| 18 | **Colores invertidos sin explicación clara** — `color_negro = #FFFFFF` | `pantalla-frigate.yml` |

---

## Estructura propuesta

```
src/
├── pantalla_helpers.h            # Orquestador principal (include de los demás + draw_calendar simplificado)
├── pantalla_config.h             # NUEVO: Constantes de layout, locale, caracteres permitidos
├── pantalla_calendar_parser.h    # NUEVO: Lógica extraída del lambda YAML (JSON, fechas, hashing)
├── pantalla_draw.h               # NUEVO: Funciones de dibujo individuales (iconos, eventos, headers)
└── pantalla_fonts.h              # NUEVO: Resolución de fuentes consolidada
```

---

## Detalle de cada paso

### Paso 1 — Crear `sources/pantalla_config.h`

Centralizar todas las constantes y valores mágicos en un solo lugar:

```cpp
#pragma once

// Dimensiones de pantalla
constexpr int SCREEN_W = 400;
constexpr int SCREEN_H = 300;

// Columnas de layout
constexpr int DATE_COL_X      = 5;
constexpr int TITLE_COL_X     = 70;
constexpr int RIGHT_MARGIN_X  = 380;
constexpr int TIME_DISPLAY_X  = 400;

// Espaciado vertical
constexpr int SPACING_WITH_LOCATION    = 34;
constexpr int SPACING_WITHOUT_LOCATION = 20;
constexpr int DAY_SEPARATOR_OFFSET     = -6;

// Límites de texto
constexpr int MAX_TITLE_CHARS   = 38;
constexpr int MAX_EVENTS        = 8;

// Cache TTL
constexpr int CACHE_TTL_SECONDS = 12 * 3600;

// Locale español
const char* const DIAS[]  = {"dom.", "lun.", "mar.", "mié.", "jue.", "vie.", "sáb."};
const char* const MESES[] = {"ene", "feb", "mar", "abr", "may", "jun",
                              "jul", "ago", "sep", "oct", "nov", "dic"};

// Caracteres permitidos (fuente glyphs)
const std::string ALLOWED_CHARS =
    "!\"#$%&'()*+,-./:;=?@°0123456789"
    "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz "
    "áéíóúÁÉÍÓÚñÑüÜ¿¡ÀÈÌÒÙàèìòùâêîôûÂÊÎÔÛäëïöÄËÏÖ";
```

**Impacto:** Elimina números mágicos de `pantalla_helpers.h` y `pantalla-frigate.yml`.

---

### Paso 2 — Crear `sources/pantalla_fonts.h`

Consolidar las 4 funciones de fuente en un patrón genérico y corregir comentarios:

```cpp
#pragma once
#include "esphome.h"

// Resolver fuente por tamaño (título)
inline esphome::display::BaseFont* get_title_font();
inline int get_title_font_size();

// Resolver fuente por tamaño (fecha)
inline esphome::display::BaseFont* get_date_font();
inline int get_date_font_size();
```

**Nota:** Unificamos los selectors de UI `ui_font_title_size` y `ui_font_date_size` en un único `ui_font_size` con valor por defecto `16`. Esto simplifica la UI y reduce duplicidad de configuraciones; las funciones de resolución mapean el valor unificado a las fuentes disponibles (p. ej. para fechas, tamaños mayores se truncarán al máximo disponible).

**Impacto:** Separa responsabilidad de resolución de fuentes del código de dibujo.

---

### Paso 3 — Crear `sources/pantalla_draw.h`

Extraer funciones de dibujo individuales y reutilizables:

```cpp
#pragma once
#include "esphome.h"

// Icono de localización (marcador de mapa)
inline void draw_location_icon(esphome::display::Display &it,
                                int x, int y,
                                esphome::Color color);

// Cabecera de fecha (día + mes + línea separadora)
inline void draw_date_header(esphome::display::Display &it,
                              int x, int y,
                              const std::string &day_num,
                              const std::string &month,
                              esphome::display::BaseFont* font,
                              int font_size,
                              esphome::Color color,
                              bool draw_separator);

// Un evento individual (título, hora, cuándo, ubicación)
inline int draw_event(esphome::display::Display &it,
                       int y, size_t index,
                       esphome::display::BaseFont* title_font,
                       int title_font_size);

// Indicador de error de conexión
inline void draw_error_indicator(esphome::display::Display &it,
                                  const std::string &error);

// Timestamp de último refresco
inline void draw_refresh_time(esphome::display::Display &it,
                               const std::string &time_str);
```

**Impacto:** Cada elemento visual tiene su propia función, testeable y reutilizable por separado.

---

### Paso 4 — Crear `sources/pantalla_calendar_parser.h`

Mover las ~290 líneas de C++ del lambda de `procesar_calendario` a funciones:

```cpp
#pragma once
#include "esphome.h"
#include "pantalla_config.h"

// Utilidades de texto
inline void trim(std::string &s);
inline void normalize(std::string &s);
inline void truncate(std::string &s, size_t max_len);

// Hash FNV-1a 64-bit (reutilizable, una sola implementación)
inline std::string fnv1a_hash(const std::string &data);

// Parsing de fecha ISO-8601 → struct tm
inline bool parse_iso_datetime(const std::string &iso, tm &result);

// Función principal de procesamiento del calendario
inline void procesar_calendario_impl();
```

**Impacto:** El lambda del YAML queda reducido a una sola línea: `procesar_calendario_impl();`

---

### Paso 5 — Simplificar `sources/pantalla_helpers.h`

Convertirlo en un orquestador que incluye los demás headers:

```cpp
#pragma once
#include "pantalla_config.h"
#include "pantalla_fonts.h"
#include "pantalla_draw.h"
#include "pantalla_calendar_parser.h"

// Verificar condiciones de refresco
inline bool check_refresh_conditions();

// Función principal de dibujo (delega a subfunciones)
inline void draw_calendar(esphome::display::Display &it);
```

`draw_calendar` quedaría así (~30 líneas):

```
1. check_refresh_conditions() → return si no procede
2. Obtener fuentes y configuración
3. Limpiar pantalla
4. draw_refresh_time()
5. Loop: para cada evento → draw_date_header() + draw_event()
6. draw_error_indicator()
```

---

### Paso 6 — Usar ancla YAML para glifos

En `pantalla-frigate.yml`, definir la cadena una sola vez:

```yaml
# Antes de la sección font:
.common_glyphs: &glyphs >-
  !"#$%&'()*+,-./:;=?@°0123456789ABCDEFGHIJKLMNOPQRSTU...

font:
  - file: "../fonts/Palatino.ttf"
    id: fuente_titulo_16
    size: 16
    glyphs: *glyphs
  - file: "../fonts/Palatino.ttf"
    id: fuente_titulo_18
    size: 18
    glyphs: *glyphs
  # ... etc
```

**Impacto:** 12 cadenas duplicadas → 1 fuente de verdad.

---

### Paso 7 — Limpieza final

- [x] Renombrar `fuente_28` → `fuente_titulo_20` (o añadir comentario `# size: 20`)
- [x] Renombrar `fuente_14` → `fuente_fecha_16` (o añadir comentario `# size: 16`)
- [x] Verificar si `fuente_8_2` se usa; si no, eliminar (eliminado)
- [x] Eliminar carpeta vacía `custom_components/ssd1680/` (eliminado)
- [x] Añadir comentarios a colores invertidos: `# e-paper invierte: ink=FFFFFF, paper=000000` (comentario presente)
- [x] Corregir variable `now_tm` duplicada en `procesar_calendario` (corregido en `pantalla_calendar_parser.h`)
- [x] Evitar persistir JSON grande en NVS: `calendario_json` ahora `restore_value: no` (previene `ESP_ERR_NVS_NOT_ENOUGH_SPACE`)
- [x] No persistir hashes procesados: `last_calendar_hash` y `last_processed_hash` ahora `restore_value: no` (se recalculan en boot)
- [x] Simplificar las 3 ramas idénticas de spacing a una sola línea:
  ```cpp
  int spacing = id(cal_location)[i].empty() ? SPACING_WITHOUT_LOCATION : SPACING_WITH_LOCATION;
  ```
- [x] Actualizar `esphome.includes` para referenciar solo `pantalla_helpers.h` (que incluye los demás)

---

## Orden de ejecución recomendado

| Fase | Pasos | Riesgo | Descripción |
|---|---|---|---|
| **Fase 1** | Pasos 1, 6, 7 | 🟢 Bajo | Constantes, ancla YAML, limpieza — sin cambio funcional |
| **Fase 2** | Pasos 2, 3 | 🟡 Medio | Extraer funciones de dibujo — compilar y probar tras cada paso |
| **Fase 3** | Paso 4 | 🔴 Alto | Mover lambda C++ a header — requiere test completo del flujo calendario |
| **Fase 4** | Paso 5 | 🟡 Medio | Simplificar orquestador — integrar todo y verificar |

> **⚠️ Importante:** Compilar y subir al dispositivo después de CADA fase para detectar regresiones a tiempo.

---

## Resultado esperado

| Métrica | Antes | Después |
|---|---|---|
| Líneas C++ en YAML | ~290 | ~1 (`procesar_calendario_impl();`) |
| Líneas de `draw_calendar` | ~170 | ~30 |
| Cadenas de glifos duplicadas | 12 | 1 |
| Números mágicos | ~25 | 0 (constantes con nombre) |
| Funciones de dibujo reutilizables | 0 | 5+ |
| Archivos .h | 1 | 5 (organizados por responsabilidad) |
