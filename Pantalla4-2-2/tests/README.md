# Tests — Usage & Guidelines ✅

Purpose
- This folder contains the unit and integration tests for the Pantalla calendar project. Tests validate pagination, parsing, drawing calculations and small integration scenarios using fixtures.

Prerequisites
- Windows (tested), Python 3.11+ installed and available on PATH.
- Git and the repository checked out.

Quick setup (PowerShell)
1. Create a virtual environment:

   ```powershell
   python -m venv .venv
   ```

2. Activate the venv (PowerShell):

   ```powershell
   .\.venv\Scripts\Activate.ps1
   ```

   (Cmd: `.\.venv\Scripts\activate.bat`)

3. Install development dependencies:

   - If `requirements-dev.txt` exists:

     ```powershell
     .\.venv\Scripts\python -m pip install -r requirements-dev.txt
     ```

   - Otherwise install the essentials:

     ```powershell
     .\.venv\Scripts\python -m pip install pytest pytest-cov
     ```

Running tests
- Run the full test suite:

  ```powershell
  .\.venv\Scripts\python -m pytest -q
  ```

- Run tests with coverage and per-file missing lines report:

  ```powershell
  .\.venv\Scripts\python -m pytest --cov=. --cov-report=term-missing -q
  ```

- Run a single test file (example):

  ```powershell
  .\.venv\Scripts\python -m pytest tests/test_device_paint_counts.py -q
  ```

- Use `-k <expr>` to select by test name, and `-s` to see printed debug output.

Fixtures & helpers
- The real calendar fixture: `tests/fixtures/calendar_real.json` (used by integration tests).
- Shared helpers live in the Python test modules (e.g., `tests/test_calc_start_index.py`). Import these helpers in new tests where appropriate.
- `tests/conftest.py` sets up `sys.path` so tests can import from the project root.

Test guidance & conventions
- Test files should be named `test_*.py` and placed under `tests/`.
- Use descriptive test names and keep tests small and focused.
- Prefer pure Python tests for algorithmic code (pagination, parsing, calc_event_height). Use integration tests with fixtures for end-to-end behavior.
- For code that interacts with hardware (waveshare driver), prefer mocking the driver component for unit tests.
- When adding tests, also update or add fixtures in `tests/fixtures/` as necessary.

CI
- The repository contains a GitHub Actions workflow that runs tests and coverage on push/PR — check `.github/workflows` for details.

Troubleshooting
- If `pytest` is not found, ensure you activated `.venv` and installed the dependencies.
- On PowerShell, you may need to run `Set-ExecutionPolicy -ExecutionPolicy RemoteSigned -Scope CurrentUser` once to allow script activation.

Contributing tests
1. Add test(s) in `tests/` following naming conventions.
2. Ensure tests pass locally in the venv.
3. Open a PR with a brief description of the new tests and the behaviour they protect.

Contact
- For questions about tests or the expected behaviour, open an issue or mention the repository maintainer in the PR.

---
*Short, practical, and focused — add tests that lock down pagination, parsing, and drawing edge cases.* ✨
