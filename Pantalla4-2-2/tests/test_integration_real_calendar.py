import json
from datetime import datetime, timezone
from test_calc_start_index import calc_start_index_py

DIAS = ["dom.", "lun.", "mar.", "mié.", "jue.", "vie.", "sáb."]
MESES = ["ene","feb","mar","abr","may","jun","jul","ago","sep","oct","nov","dic"]


def iso_to_date_components(iso):
    # parse ISO with offset, get local date components
    dt = datetime.fromisoformat(iso)
    # normalize to local date (keep offset)
    return dt


def make_event_record(obj, now_date):
    start_dt = iso_to_date_components(obj['start'])
    end_dt = iso_to_date_components(obj['end']) if 'end' in obj else None

    # compute day string (C 'tm_wday' uses Sunday=0)
    py_w = start_dt.weekday()  # Monday=0
    c_w = (py_w + 1) % 7
    day_str = f"{DIAS[c_w]} {start_dt.day} {MESES[start_dt.month-1]}"

    # time string
    if end_dt is not None:
        time_str = f"{start_dt.strftime('%H:%M')}-{end_dt.strftime('%H:%M')}"
    else:
        time_str = "Todo el día"

    # when string relative to now_date
    days = (start_dt.date() - now_date.date()).days
    if days < 0:
        return None
    if days == 0:
        when = "(hoy)"
    elif days == 1:
        when = "(mañana)"
    elif days < 30:
        when = f"(en {days} días)"
    else:
        when = "(+1 mes)"

    ev = {
        'day': day_str,
        'title': obj.get('summary', ''),
        'time': time_str,
        'location': obj.get('location', ''),
        'when': when
    }
    return ev


def test_integration_real_calendar():
    with open('tests/fixtures/calendar_real.json', 'r', encoding='utf-8') as f:
        arr = json.load(f)

    # Use same 'now' as device logs: 2026-02-08 12:00
    now = datetime(2026, 2, 8, 12, 0, 0, tzinfo=timezone.utc).astimezone()

    events = []
    for o in arr:
        r = make_event_record(o, now)
        if r:
            events.append(r)

    # Use sim_shrink=32 and density=1 like runtime logs
    sim_shrink = 32
    density = 1
    title_font_size = 16

    start1 = calc_start_index_py(1, events, title_font_size, density, sim_shrink)
    start2 = calc_start_index_py(2, events, title_font_size, density, sim_shrink)

    print(f"Computed starts: device1={start1} device2={start2}")

    assert start1 == 0
    assert start2 == 7  # matches observed log: draw_calendar: device=2 start_idx=7
