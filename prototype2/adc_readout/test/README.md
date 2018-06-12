## Testing check list

### Coverage

* Add coverage flags to _event-formation-unit/cmake/CompilerConfig.cmake_, i.e.

```
set(CMAKE_EXE_LINKER_FLAGS "-fprofile-instr-generate -fcoverage-mapping")
set(EXTRA_CXX_FLAGS "-Werror -Wall -Wpedantic -Wextra -fprofile-instr-generate -fcoverage-mapping")
```

* Compile and run unit tests.
* Process data from application run: `llvm-profdata-mp-devel merge -sparse default.profraw -o default.profdata`
* Convert coverage data to something useful: `llvm-cov-mp-devel  show ./AdcReadoutTest  -instr-profile=default.profdata -format=html -o coverage`

### Clang-tidy
Run CMake as follows:

```
cmake .. -DCMAKE_CXX_CLANG_TIDY:STRING="clang-tidy-mp-devel;-checks=*,-cppcoreguidelines-pro-type-reinterpret-cast,-cppcoreguidelines-pro-bounds-pointer-arithmetic,-cppcoreguidelines-pro-type-vararg,-cppcoreguidelines-pro-bounds-array-to-pointer-decay"
```
