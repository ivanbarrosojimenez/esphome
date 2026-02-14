import json
from datetime import datetime, timezone
from test_calc_start_index import calc_event_height, calc_start_index_py, SCREEN_H, VISIBILITY_THRESHOLD, DEVICE1_VISIBILITY_THRESHOLD
from test_integration_real_calendar import make_event_record


def count_painted(start, device_number, events, title_font_size=16, density=1):
    y = 0
    cnt = 0
    total = len(events)
    for i in range(start, total):
        next_same = (i + 1 < total and events[i]['day'] == events[i + 1]['day'])
        event_h = calc_event_height(events[i]['title'], events[i]['location'], next_same, title_font_size)
        # The draw path uses the same visibility threshold used for device 1 in logs;
        # use DEVICE1_VISIBILITY_THRESHOLD for all devices to match observed behavior.
        vis_frac = DEVICE1_VISIBILITY_THRESHOLD
        required = int((event_h * vis_frac) + 0.999)
        if y + required > SCREEN_H:
            break
        cnt += 1
        y += event_h + density
    return cnt


def test_device_paint_counts():
    with open('tests/fixtures/calendar_real.json', 'r', encoding='utf-8') as f:
        arr = json.load(f)

    # Use same 'now' as device logs: 2026-02-08 12:00
    now = datetime(2026, 2, 8, 12, 0, 0, tzinfo=timezone.utc).astimezone()

    events = []
    for o in arr:
        r = make_event_record(o, now)
        if r:
            events.append(r)

    sim_shrink = 32
    start1 = calc_start_index_py(1, events, 16, 1, sim_shrink)
    start2 = calc_start_index_py(2, events, 16, 1, sim_shrink)

    # sanity checks on starts
    assert start1 == 0
    assert start2 == 7

    # Compute painted counts indirectly from start indices (partition boundaries)
    start3 = calc_start_index_py(3, events, 16, 1, sim_shrink)
    c1 = start2 - start1
    c2 = start3 - start2

    assert c1 == 7
    assert c2 == 6
