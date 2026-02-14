import sys, os
sys.path.insert(0, os.path.abspath(os.path.join(os.path.dirname(__file__), '..')))
from tests.test_calc_start_index import calc_start_index_py
import json
from datetime import datetime, timezone
DIAS = ["dom.", "lun.", "mar.", "mié.", "jue.", "vie.", "sáb."]
MESES = ["ene","feb","mar","abr","may","jun","jul","ago","sep","oct","nov","dic"]

with open('tests/fixtures/calendar_real.json','r',encoding='utf-8') as f:
    arr = json.load(f)
now = datetime(2026,2,8,12,0,0, tzinfo=timezone.utc).astimezone()

def iso_to_date_components(iso):
    return datetime.fromisoformat(iso)

def make_event_record(obj, now_date):
    start_dt = iso_to_date_components(obj['start'])
    end_dt = iso_to_date_components(obj['end']) if 'end' in obj else None
    py_w = start_dt.weekday()
    c_w = (py_w + 1) % 7
    day_str = f"{DIAS[c_w]} {start_dt.day} {MESES[start_dt.month-1]}"
    if end_dt is not None:
        time_str = f"{start_dt.strftime('%H:%M')}-{end_dt.strftime('%H:%M')}"
    else:
        time_str = "Todo el día"
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
    return {'day': day_str, 'title': obj.get('summary',''), 'time': time_str, 'location': obj.get('location',''), 'when': when}

events = []
for o in arr:
    r = make_event_record(o, now)
    if r:
        events.append(r)

density = 1
title_font_size = 16
for sim_shrink in range(28,35):
    print('\n--- sim_shrink =', sim_shrink)
    for device in range(1,5):
        start = calc_start_index_py(device, events, title_font_size, density, sim_shrink)
        print(f"device={device} -> start_index={start}")
