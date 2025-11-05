
[![License](https://img.shields.io/badge/License-Apache%202.0-blue.svg)](https://github.com/ecmwf/metkit/blob/develop/LICENSE)
[![Static Badge](https://github.com/ecmwf/codex/raw/refs/heads/main/Project%20Maturity/graduated_badge.svg)](https://github.com/ecmwf/codex/raw/refs/heads/main/Project%20Maturity#graduated))

> \[!IMPORTANT\]
> This software is **Graduated** and subject to ECMWF's guidelines on [Software Maturity](https://github.com/ecmwf/codex/raw/refs/heads/main/Project%20Maturity).

# metkit

## Requirements

Runtime dependencies:

- eckit -- http://github.com/ecmwf/eckit

Build dependencies:

- CMake --- For use and installation see http://www.cmake.org/
- ecbuild --- ECMWF library of CMake macros ()

## Installation

metkit employs an out-of-source build/install based on CMake.

Make sure ecbuild is installed and the ecbuild executable script is found ( `which ecbuild` ).

Now proceed with installation as follows:

```bash
# Environment --- Edit as needed
srcdir=$(pwd)
builddir=build
installdir=$HOME/local  

# 1. Create the build directory:
mkdir $builddir
cd $builddir

# 2. Run CMake
ecbuild --prefix=$installdir -- -DECKIT_PATH=<path/to/eckit/install> $srcdir

# 3. Compile / Install
make -j10
make install
```

## Release

Config files should be updated before releasing a new version.
Please refer to the documentation available on [Confluence](https://confluence.ecmwf.int/display/METK/MetKit+configuration)


## License

Copyright 2021 European Centre for Medium-Range Weather Forecasts (ECMWF)

Licensed under the Apache License, Version 2.0 (the "License"); you may not use this file except in compliance with the License. You may obtain a copy of the License at
> http://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software distributed under the License is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied. See the License for the specific language governing permissions and limitations under the License.

In applying this licence, ECMWF does not waive the privileges and immunities granted to it by virtue of its status as an intergovernmental organisation nor does it submit to any jurisdiction.