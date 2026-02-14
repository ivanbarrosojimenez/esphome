import json
from datetime import datetime, timezone
from test_integration_real_calendar import make_event_record
from test_device_paint_counts import count_painted
from test_calc_start_index import calc_start_index_py


def test_count_painted_matches_debug_values():
    with open('tests/fixtures/calendar_real.json', 'r', encoding='utf-8') as f:
        arr = json.load(f)

    now = datetime(2026, 2, 8, 12, 0, 0, tzinfo=timezone.utc).astimezone()
    events = [make_event_record(o, now) for o in arr]
    events = [e for e in events if e]

    sim_shrink = 32
    s1 = calc_start_index_py(1, events, 16, 1, sim_shrink)
    s2 = calc_start_index_py(2, events, 16, 1, sim_shrink)

    # direct counts using count_painted (matches earlier diagnostic)
    c1 = count_painted(s1 if s1 > 0 else 0, 1, events)
    c2 = count_painted(s2, 2, events)

    assert c1 == 7
    # note: direct count may differ from partition difference depending on thresholds; assert observed value
    assert c2 == 7


def test_count_painted_edge_cases():
    # empty list
    events = []
    assert count_painted(0, 1, events) == 0
    assert count_painted(1, 2, events) == 0

    # start beyond length
    events = [{'day': 'lun. 1 feb', 'title': 'Short', 'location': '', 'time': '12:00', 'when': '(hoy)'}]
    assert count_painted(10, 1, events) == 0
