# Simple python tests that mirror calc_start_index logic to catch off-by-one errors

SCREEN_H = 300
SPACING_WITH_LOCATION = 34
SPACING_WITHOUT_LOCATION = 20
VISIBILITY_THRESHOLD = 0.9
DEVICE1_VISIBILITY_THRESHOLD = 0.8

# mimic split_two_lines by truncating at max chars (simple approach)
def calc_event_height(title, location, next_same_day, title_font_size):
    # split into up to two lines by length
    max_chars = 38
    words = title.split(' ')
    line1 = ""
    line2 = ""
    for w in words:
        if len(line1) + len(w) + 1 <= max_chars:
            line1 = (line1 + ' ' + w).strip()
        else:
            line2 = (line2 + ' ' + w).strip()
    title_height_single = title_font_size + 2
    title_height = title_height_single if not line2 else title_height_single * 2
    spacing = SPACING_WITH_LOCATION if location else SPACING_WITHOUT_LOCATION
    if next_same_day:
        spacing = max(0, spacing - 4)
    return title_height + spacing


def calc_start_index_py(device_number, events, title_font_size, density, sim_shrink):
    if device_number <= 1:
        return 0
    start_index = 0
    total = len(events)
    last_painted_index = -1
    for d in range(1, device_number):
        y = 0
        last_day = ''
        painted = []
        for i in range(start_index, total):
            if events[i]['day'] != last_day:
                last_day = events[i]['day']
            next_same = (i+1 < total and events[i]['day'] == events[i+1]['day'])
            event_h = calc_event_height(events[i]['title'], events[i]['location'], next_same, title_font_size)
            # Do not apply sim_shrink when simulating the first device (d == 1)
            threshold = max(0, SCREEN_H - (sim_shrink if d != 1 else 0))
            vis_frac = DEVICE1_VISIBILITY_THRESHOLD if d == 1 else VISIBILITY_THRESHOLD
            required = int((event_h * vis_frac) + 0.999)
            if y + required > threshold:
                break
            y += event_h + density
            painted.append(i)
            start_index = i + 1
            last_painted_index = painted[-1]
            if start_index >= total:
                start_index = max(start_index, last_painted_index + 1)
                return start_index
        if start_index == 0:
            break
    if last_painted_index >= 0:
        corrected = last_painted_index + 1
        if corrected > start_index:
            start_index = corrected
    # post-check
    test_start = start_index
    attempts = 0
    while test_start > 0 and attempts < 6:
        y_test = 0
        painted_test = 0
        for i in range(test_start, total):
            next_same = (i+1 < total and events[i]['day'] == events[i+1]['day'])
            event_h = calc_event_height(events[i]['title'], events[i]['location'], next_same, title_font_size)
            vis_frac = DEVICE1_VISIBILITY_THRESHOLD if device_number == 1 else VISIBILITY_THRESHOLD
            required = int((event_h * vis_frac) + 0.999)
            if y_test + required > SCREEN_H:
                break
            painted_test += 1
            y_test += event_h + density
            if painted_test > 0:
                break
        if painted_test > 0:
            break
        test_start -= 1
        attempts += 1
    if test_start != start_index:
        start_index = test_start
    return start_index


# TEST CASES
if __name__ == '__main__':
    events = []
    # create 14 events spaced across days and some with location
    for i in range(14):
        day = f"lun. {1 + i//3} feb"
        events.append({'day': day, 'title': f"Event {i+1} title long enough to wrap maybe", 'time': '12:00', 'location': '' if i%2==0 else 'Loc', 'when': '(en 1 día)'})

    # Test different sim_shrink values and device numbers
    for sim_shrink in [0, 10, 32]:
        for device in [1,2,3]:
            start = calc_start_index_py(device, events, 16, 1, sim_shrink)
            print(f"sim_shrink={sim_shrink} device={device} -> start_index={start}")

    # Simple assertions for plausibility (non-regression)
    assert calc_start_index_py(1, events, 16, 1, 0) == 7 or calc_start_index_py(1, events, 16, 1, 0) == 8
    print("Basic plausibility checks passed.")
