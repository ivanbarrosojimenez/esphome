from test_calc_start_index import calc_start_index_py


def make_event(day_index, i, with_loc=False):
    day = f"lun. {1 + day_index} feb"
    title = f"Event {i} {'longtitle ' * (i%3)}"
    return {'day': day, 'title': title.strip(), 'location': 'Loc' if with_loc else '', 'time': '12:00', 'when': '(en 1 días)'}


def test_sim_shrink_not_applied_to_device1():
    # Build a list of events that fill the screen
    events = [make_event(i//2, i, with_loc=(i%2==0)) for i in range(12)]

    start1_s0 = calc_start_index_py(1, events, 16, 1, 0)
    start1_s50 = calc_start_index_py(1, events, 16, 1, 50)

    # sim_shrink must NOT affect device 1 start
    assert start1_s0 == start1_s50


def test_partition_no_overlap_and_increasing():
    events = [make_event(i//2, i, with_loc=(i%2==0)) for i in range(18)]

    starts = [calc_start_index_py(d, events, 16, 1, 32) for d in range(1, 5)]

    # starts must be non-decreasing and start1==0
    assert starts[0] == 0
    for a, b in zip(starts, starts[1:]):
        assert b >= a

    total = len(events)
    # final start should be <= total
    assert starts[-1] <= total


def test_empty_and_single_event():
    # empty list
    events = []
    assert calc_start_index_py(1, events, 16, 1, 32) == 0
    assert calc_start_index_py(2, events, 16, 1, 32) == 0

    # single event
    events = [make_event(0, 0, with_loc=False)]
    assert calc_start_index_py(1, events, 16, 1, 32) == 0
    # second device should start after the only event
    assert calc_start_index_py(2, events, 16, 1, 32) >= 1


def test_many_same_day_events_partitioning():
    # 12 events the same day
    events = [make_event(0, i, with_loc=(i%2==0)) for i in range(12)]
    s1 = calc_start_index_py(1, events, 16, 1, 32)
    s2 = calc_start_index_py(2, events, 16, 1, 32)
    s3 = calc_start_index_py(3, events, 16, 1, 32)

    # ensure no overlap (partitioning) and coverage
    assert s1 == 0
    assert s2 >= s1
    assert s3 >= s2
    assert s3 <= len(events)


def test_post_check_decrements_with_large_font():
    # First two small events, then two huge events that will not fit under high font size
    small = make_event(0, 0, with_loc=False)
    small2 = make_event(0, 1, with_loc=False)
    huge = make_event(1, 2, with_loc=False)
    huge2 = make_event(1, 3, with_loc=False)
    events = [small, small2, huge, huge2]

    # With normal font, start index will typically be >0
    s_normal = calc_start_index_py(2, events, 16, 1, 10)

    # With huge font, the post-check should detect that starting at same index would paint 0
    s_large = calc_start_index_py(2, events, 200, 1, 10)

    assert 0 <= s_large <= len(events)
    # ensure correction happened (start reduced) when using large font
    assert s_large <= s_normal


def test_start_index_zero_when_first_event_too_tall():
    # single huge event that cannot be painted by the first simulated device
    huge = make_event(0, 0, with_loc=False)
    events = [huge]

    # With very large font the first simulated device will paint nothing
    s = calc_start_index_py(2, events, 500, 1, 0)
    assert s == 0


def test_device_counts_consistent_with_partition():
    events = [make_event(i//2, i, with_loc=(i%2==0)) for i in range(21)]
    s1 = calc_start_index_py(1, events, 16, 1, 32)
    s2 = calc_start_index_py(2, events, 16, 1, 32)
    s3 = calc_start_index_py(3, events, 16, 1, 32)

    c1 = s2 - s1
    c2 = s3 - s2

    # basic sanity: painted counts positive (or zero) and sum <= total
    assert c1 >= 0
    assert c2 >= 0
    assert c1 + c2 <= len(events)