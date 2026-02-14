from test_calc_start_index import calc_event_height


def test_calc_event_height_wrap_and_spacing():
    # short title, no location, not same day
    h1 = calc_event_height('Short title', '', False, 16)
    # single line: title_height_single + spacing_without_location
    assert h1 == (16 + 2) + 20

    # long title causing wrap
    long_title = 'This is a very long title ' * 3
    h2 = calc_event_height(long_title, '', False, 16)
    # double line height + spacing
    assert h2 == ((16 + 2) * 2) + 20

    # next_same_day reduces spacing by 4
    h3 = calc_event_height('Short title', 'Loc', True, 16)
    assert h3 == (16 + 2) + (34 - 4)

    # ensure spacing never negative
    h4 = calc_event_height('Short', '', True, 16)
    assert h4 >= (16 + 2)

    # exact boundary: title with length exactly max chars (single long word) will be placed in second line by our simple split logic
    max_chars_title = 'A' * 38
    h5 = calc_event_height(max_chars_title, '', False, 16)
    # double line expected because the splitting algorithm breaks long words into second line
    assert h5 == ((16 + 2) * 2) + 20

    # really long single word (no spaces) should be treated as a single long title -> still counts as 2 lines in our approximation
    long_word = 'A' * 120
    h6 = calc_event_height(long_word, 'Loc', False, 16)
    assert h6 >= ((16 + 2) * 2) + 34
