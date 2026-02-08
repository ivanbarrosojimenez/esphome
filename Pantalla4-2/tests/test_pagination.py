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