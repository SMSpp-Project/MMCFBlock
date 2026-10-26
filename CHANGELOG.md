# Changelog

All notable changes to this project will be documented in this file.

The format is based on [Keep a Changelog](https://keepachangelog.com/en/1.0.0/),
and this project adheres to [Semantic Versioning](https://semver.org/spec/v2.0.0.html).

## [Unreleased]

### Added

- the makefiles build the C++20 module `smspp.mmcfblock` when `SMSppMOD` is
  not empty (e.g., `make SMSppMOD=1`), with clang++ 16 or later [see
  `lib/makefile-mod` of SMS++]

- the C++20 module `smspp.mmcfblock` (`src/MMCFBlock.cppm`), which exports
  every name that the headers declare at namespace scope and re-exports
  `smspp.binaryknapsackblock` and `smspp.mcfblock`, so that a source file
  can write `import smspp.mmcfblock;` in place of the `#include` of the
  headers; it is built with the CMake option `SMSpp_BUILD_MODULE`, as the
  target `SMS++::MMCFBlock_module`

### Changed

- the pipeline builds the module also with its makefiles, after CMake, and
  runs on a change of the makefiles too [see `ci/makefile` of the umbrella]

- `SMSpp_BUILD_MODULE` no longer stops the configuration when the C++20
  module cannot be built, i.e., with a CMake older than 3.28, a generator
  other than Ninja or Visual Studio, a compiler other than Clang 16 (with
  `clang-scan-deps`) or MSVC 19.34, or a module it imports that is not
  there: the option is turned off with a warning that says why, and the
  build goes on with the headers [see `cmake/CheckModule.cmake` of SMS++]

### Fixed

- `makefile` compiles with the -I of MCFBlock and BinaryKnapsackBlock,
  which it takes in input as the other basic makefiles do: it relied on
  `makefile-s` adding them to `MMCFINC` afterwards, so that whoever
  included it with MCFBlock and BinaryKnapsackBlock already there (e.g.,
  next to CapacitatedFacilityLocationBlock) did not compile


## [0.5.0] - 2026-10-09

### Added

- `MMCFNetworkDesignBlock`, the multicommodity network design problem on an
  `MMCFBlock`, either monolithic (the `MMCFBlock` as a sub-Block with binary
  design variables) or in Benders form (the design variables, the value
  variable and its cuts, the flows being a sub-Block solved by the Solver of
  its `BlockSolverConfig`, from which the Benders cut of a design is
  computed by weak duality)

- `MMCFBlock::set_design_variables()`, which makes the mutual capacity
  constraints read `sum_k x^k_j <= u_j y_j` with the given variables (and,
  if asked, the forcing constraints `x^k_j <= u^k_j y_j`, `u^k_j` being the
  smallest among the individual capacity, `u_j` and the demand of `k`), and
  the getters of the data (`get_C()`, `get_U()`, `get_UTot()`, `get_B()`,
  `get_F()`, `get_Startn()`, `get_Endn()`), of the capacity of a design and
  of the copy of the Block with the capacities scaled

- `chg_demands()`, registered in the method factory so that a `DataMapping`
  can call it, which changes the demand of a range of commodities by
  scaling their node deficits, the flow conservation constraints or the
  deficits of the `MCFBlock` of the commodity following, and `get_demand()`,
  the demand of a commodity; with them a `TwoStageStochasticBlock` can make
  the demands uncertain

- the Block has an `FRealObjective` of its own, with no Variable, besides
  those of its sub-Block, and gives a `ColVariableSolution` as its
  `Solution`, so that it can be the Block of a `LagBFunction`, e.g., as the
  scenario of a `TwoStageStochasticBlock` decomposed by a
  `LagrangianDualSolver`

### Changed

- whoever links the module keeps it: the classes of a module register
  themselves in the factory from a static initialiser, and a linker that
  drops what looks unused takes the registration away with it, so the target
  now tells whoever links it to keep the symbol that forces the module in,
  and on ELF, where naming the symbol is not enough, the library as a whole

### Fixed

- `MMCFBlock::deserialize()` on a Block already loaded destroys the old data
  in place instead of through a temporary, checks the number of mutual
  constraints, and keeps it in the member; the common initialization clears
  the vectors before filling them, and the scalar members start at 0

- on macOS a program linking the module lost the classes the module
  registers in the factories when the linker dropped the library, as it
  does under `-dead_strip_dylibs`, which conda sets: the target now asks the
  linker for the symbol that forces the module in (`-u`), which ld64,
  unlike the ELF linker, counts as a use of the library

## [0.4.0] - 2026-09-12

### Changed

- which structure the MMCFBlock has, i.e., one MCFBlock per commodity or one
  BinaryKnapsackBlock per arc, is now decided by `set_structure()`, since a
  MMCF instance has no sub-Block of its own and the tree is entirely a
  modelling choice; `generate_abstract_variables()` still makes the choice out
  of the Configuration of the Variable if nobody has made it before, and
  throws if the two disagree

- the version of the module is the git tag of its repository, or the
  VERSION.txt of a release tarball, and the shared library carries it: its
  SONAME is major.minor while the major is 0, and it is installed with an
  RPATH relative to itself, so that an installed tree keeps working wherever
  it is moved

## [0.3.1] - 2024-02-28

### Added

- `set_dual()` and `set_potential()` functions

### Changed

- adapted to new CMake / makefile organisation

### Fixed

- bunch of minor fixes

## [0.3.0] - 2022-06-29

### Added

- `serialize()` and `deserialize()` functions

- `get_flow()` and `get_potential()` functions

## [0.2.0] - 2021-12-07

### Added

- Lagrangian Knapsack Relaxation

- possibility of Strong Forcing Constraint in the Knapsack Relaxation

- functions to get flow

### Fixed

- several fixes

## [0.1.0] - 2021-02-05

### Added

- First test release.

[Unreleased]: https://gitlab.com/smspp/mmcfblock/-/compare/0.5.0...develop
[0.5.0]: https://gitlab.com/smspp/mmcfblock/-/compare/0.4.0...0.5.0
[0.4.0]: https://gitlab.com/smspp/mmcfblock/-/compare/0.3.1...0.4.0
[0.3.1]: https://gitlab.com/smspp/mmcfblock/-/compare/0.3.0...0.3.1
[0.3.0]: https://gitlab.com/smspp/mmcfblock/-/compare/0.2.0...0.3.0
[0.2.0]: https://gitlab.com/smspp/mmcfblock/-/compare/0.1.0...0.2.0
[0.1.0]: https://gitlab.com/smspp/mmcfblock/-/tags/0.1.0
