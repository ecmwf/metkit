This is just a placeholder for proper python interface of metkit -- for now we just showcase `findlibs`-based finding of `.so`, and loading via cffi.

To demonstrate functionality, install `eckit` and `metkit` binary wheels in your venv, and then you can `pip install -e . && python -c 'import metkit; metkit.version()`.

To be done:
1. proper cffi setup
2. wheel building (note this is **not** a wheel containing the metkit cpp lib! That happens in `../metkitlib`, and contains no python code)
