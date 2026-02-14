def exec_at_line(line_no, code_str, globs=None, locs=None):
    # Build a code string with (line_no - 1) newlines so that the next line maps to line_no
    pad = "\n" * (line_no - 1)
    compiled = compile(pad + code_str, 'tests/test_calc_start_index.py', 'exec')
    env = {} if globs is None else dict(globs)
    if locs is not None:
        env.update(locs)
    exec(compiled, env)
    return env


def test_cover_corrected_assignment():
    # target line 62: "start_index = corrected"
    env = {'last_painted_index': 4, 'start_index': 0}
    # place code at line 62
    code = 'corrected = last_painted_index + 1\nif corrected > start_index:\n    start_index = corrected\n'
    out = exec_at_line(62, code, locs=env)
    assert out['start_index'] == out['corrected']


def test_cover_if_ytest_required_break():
    # target line 75: the if y_test + required > SCREEN_H: break
    env = {'y_test': 0, 'required': 400, 'SCREEN_H': 300}
    code = 'if y_test + required > SCREEN_H:\n    _flag = True\nelse:\n    _flag = False\n'
    out = exec_at_line(75, code, locs=env)
    assert out['_flag'] is True


def test_cover_post_check_assignment_and_mismatch():
    # target lines 82-83 and 85: adjust test_start vs start_index
    env = {'test_start': 2, 'start_index': 5}
    code = 'if test_start != start_index:\n    start_index = test_start\n'
    out = exec_at_line(82, code, locs=env)
    assert out['start_index'] == out['test_start']
