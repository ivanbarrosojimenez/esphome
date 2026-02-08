import pytest
from datetime import datetime, timezone, timedelta
from test_integration_real_calendar import make_event_record


def test_all_day_event():
    now = datetime(2026, 2, 8, 12, 0, 0, tzinfo=timezone.utc).astimezone()
    obj = {'start': '2026-02-09'}
    ev = make_event_record(obj, now)
    assert ev is not None
    assert ev['time'] == 'Todo el día'
    assert ev['when'] == '(mañana)'



def test_timezone_offset_day_boundary():
    # Start at 23:00 +01:00 is same local day for Europe/Madrid
    now = datetime(2026, 2, 8, 12, 0, 0, tzinfo=timezone.utc).astimezone()
    obj = {'start': '2026-02-08T23:00:00+01:00', 'end': '2026-02-08T23:30:00+01:00', 'summary': 'Late meeting'}
    ev = make_event_record(obj, now)
    assert ev is not None
    assert '23:00-23:30' in ev['time']
    assert '(hoy)' in ev['when'] or '(en 0' in ev['when']


def test_past_event_filtered():
    now = datetime(2026, 2, 8, 12, 0, 0, tzinfo=timezone.utc).astimezone()
    obj = {'start': '2026-02-07T10:00:00+01:00', 'summary': 'Yesterday event'}
    ev = make_event_record(obj, now)
    assert ev is None


def test_missing_fields_and_long_range():
    now = datetime(2026, 2, 8, 12, 0, 0, tzinfo=timezone.utc).astimezone()
    obj = {'start': '2026-03-20T09:00:00+01:00'}  # >30 days
    ev = make_event_record(obj, now)
    assert ev is not None
    assert ev['title'] == ''
    assert ev['location'] == ''
    assert ev['when'] == '(+1 mes)'
