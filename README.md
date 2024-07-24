# LibMWR

LibMWR contains types and utilities shared between all MachineWare projects.

[![Build Status](https://github.com/machineware-gmbh/mwr/actions/workflows/cmake.yml/badge.svg?branch=main)](https://github.com/machineware-gmbh/mwr/actions/workflows/cmake.yml)
[![ASAN Status](https://github.com/machineware-gmbh/mwr/actions/workflows/asan.yml/badge.svg?branch=main)](https://github.com/machineware-gmbh/mwr/actions/workflows/asan.yml)
[![Lint Status](https://github.com/machineware-gmbh/mwr/actions/workflows/lint.yml/badge.svg?branch=main)](https://github.com/machineware-gmbh/mwr/actions/workflows/lint.yml)
[![Style Status](https://github.com/machineware-gmbh/mwr/actions/workflows/style.yml/badge.svg?branch=main)](https://github.com/machineware-gmbh/mwr/actions/workflows/style.yml)
[![Nightly Status](https://github.com/machineware-gmbh/mwr/actions/workflows/nightly.yml/badge.svg?branch=main)](https://github.com/machineware-gmbh/mwr/actions/workflows/nightly.yml)
[![Coverage Status](https://github.com/machineware-gmbh/mwr/actions/workflows/coverage.yml/badge.svg?branch=main)](https://github.com/machineware-gmbh/mwr/actions/workflows/coverage.yml)

----
## Installation

Clone repository and update submodules if you want to build unit tests:
```
git clone https://github.com/machineware-gmbh/mwr.git --recursive
cd mwr
git submodule update --init
```
Building `mwr` requires `cmake >= 3.11`. During configuration, you must state
whether to build the unit tests and the example programs:
* `-DMWR_BUILD_TESTS=[ON|OFF]`: build unit tests (default `OFF`)
* `-DMWR_LINTER=<string>`: linter program to use (default `<empty>`)
```
mkdir -p BUILD/RELEASE/BUILD
cd BUILD/RELEASE/BUILD
cmake -DCMAKE_BUILD_TYPE=RELEASE -DCMAKE_INSTALL_PREFIX=.. \
      -DMWR_BUILD_TESTS=ON
make -j 4
make test
make install
```
If everything went well, the following artifacts will have been installed:
* mwr headers in `BUILD/RELEASE/include`
* mwr library in `BUILD/RELEASE/lib`

----
## Integration
You can use `mwr` as a submodule within `cmake` projects. For example:
```
add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/mwr)
...
target_link_libraries(my_target mwr)
```

----
## License

This project is licensed under the Apache-2.0 license - see the
[LICENSE](LICENSE) file for details.

