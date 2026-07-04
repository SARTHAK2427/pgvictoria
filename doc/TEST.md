# Testing

This document describes the testing infrastructure for the `pgvictoria` project, how to run tests locally, and how to write new unit tests using the Minimal C Test Framework (MCTF).

## Running Tests

### Dependencies

To build the project and run the tests, you need the following standard compilation and library packages installed on your Linux system (e.g., Ubuntu, Fedora, or WSL):

* `gcc` and/or `clang`
* `cmake` and `make`
* `libev-dev`
* `libssl-dev`
* `libxml2-dev`
* `python3-docutils` and `doxygen` (optional, for docs)

### Compiling and Running

To compile the codebase with test checks enabled and run all unit tests, execute the test runner script:

```bash
./test/check.sh
```

Alternatively, if you are working directly inside your `build/` directory:

```bash
# Rebuild the executable target
make -j$(nproc)

# Run the test suite
../test/check.sh
```

All test outcomes, logging slices, and visual summary reports will be generated under `/tmp/pgvictoria-test/`.

---

## MCTF (Minimal C Test Framework)

`pgvictoria` uses MCTF to organize and run tests. It is built for simplicity, sequential execution, and process-level crash safety.

### What MCTF Provides:
* **Automatic test registration**: Tests register themselves dynamically using constructor attributes (`MCTF_TEST`).
* **Module structure**: Module names are automatically extracted from source filenames (e.g., `test_postgresql.c` becomes the `postgresql` module).
* **Safe assertions**: Assert macros with optional printf-style diagnostic logging (e.g., `MCTF_ASSERT`, `MCTF_ASSERT_INT_EQ`, `MCTF_ASSERT_PTR_NONNULL`).
* **Log-Error gating**: Captures a test-specific slice of `pgvictoria.log` and fails any positive test case if unexpected `ERROR` logs are emitted.
* **Performance limits**: Configures strict execution timeouts for performance-critical logic via `MCTF_TEST_MAX`.

### Assertion Usage Example:
```c
// Basic check
MCTF_ASSERT(value == expected, cleanup);

// With custom error message
MCTF_ASSERT(value == expected, cleanup, "value was invalid");

// With printf-style formatting
MCTF_ASSERT(value == expected, cleanup, "got %d, expected %d", value, expected);
```

---

## Adding Testcases

1. Create or open a test file under the `test/testcases/` directory (e.g., `test/testcases/test_my_feature.c`).
2. Include the test header:
   ```c
   #include <mctf.h>
   #include <tscommon.h>
   ```
3. Declare your test using the `MCTF_TEST()` macro:
   ```c
   MCTF_TEST(test_my_feature_behavior)
   {
      int result = my_feature_execute();
      MCTF_ASSERT(result == 0, cleanup, "execution failed with code %d", result);

   cleanup:
      MCTF_FINISH();
   }
   ```
4. Run code formatting on your files before staging:
   ```bash
   ./clang-format.sh
   ```
