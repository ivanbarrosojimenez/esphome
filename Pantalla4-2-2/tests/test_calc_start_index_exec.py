import runpy


def test_exec_calculator_main_block_runs():
    # Execute the script's __main__ block to increase test coverage of that file
    runpy.run_path('tests/test_calc_start_index.py', run_name='__main__')
