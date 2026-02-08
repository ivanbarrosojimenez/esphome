import re


def test_font_selects_present_in_yaml():
    with open('pantalla-frigate.yml','r',encoding='utf-8') as f:
        s = f.read()

    assert 'name: "UI tamaño fuente"' in s
    assert 'initial_option: "16"' in s
    # bold options available
    assert '12 bold' in s
    assert '20 bold' in s

    # active screen select exists
    assert 'name: "Pantalla número activa"' in s
    # initial option is stored as string (quoted)
    assert 'initial_option: "1"' in s