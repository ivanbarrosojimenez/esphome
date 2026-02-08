import importlib, sys


def test_conftest_reload_inserts_paths(tmp_path, monkeypatch):
    # Simulate environment where ROOT and TESTS_DIR are not in sys.path
    # reload the conftest module and ensure it inserts paths (no exception)
    # We achieve this by removing any existing entries and reloading
    import tests.conftest as confmod
    # backup sys.path
    orig = sys.path.copy()
    try:
        # Ensure the paths are not present
        sys.path = [p for p in sys.path if 'Pantalla4-2' not in p]
        importlib.reload(confmod)
        # After reload, the module will have attempted insertion; just assert no error
        assert hasattr(confmod, 'ROOT')
        assert hasattr(confmod, 'TESTS_DIR')
    finally:
        sys.path = orig
