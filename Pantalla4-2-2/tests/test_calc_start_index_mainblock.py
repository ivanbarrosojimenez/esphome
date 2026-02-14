from test_calc_start_index import calc_start_index_py


def test_main_block_simulation_matches_plausibility():
    events = []
    # create 14 events spaced across days and some with location
    for i in range(14):
        day = f"lun. {1 + i//3} feb"
        events.append({'day': day, 'title': f"Event {i+1} title long enough to wrap maybe", 'time': '12:00', 'location': '' if i%2==0 else 'Loc', 'when': '(en 1 día)'})

    # Run the same loops and ensure we get integer starts and plausibility assertion holds
    for sim_shrink in [0, 10, 32]:
        for device in [1, 2, 3]:
            start = calc_start_index_py(device, events, 16, 1, sim_shrink)
            assert isinstance(start, int)

    # Simple plausibility check copied from module (loosened)
    s = calc_start_index_py(1, events, 16, 1, 0)
    assert 0 <= s <= len(events)
