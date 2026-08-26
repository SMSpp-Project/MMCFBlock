# Changelog

All notable changes to this project will be documented in this file.

The format is based on [Keep a Changelog](https://keepachangelog.com/en/1.0.0/),
and this project adheres to [Semantic Versioning](https://semver.org/spec/v2.0.0.html).

## [Unreleased]

### Changed

- which structure the MMCFBlock has, i.e., one MCFBlock per commodity or one
  BinaryKnapsackBlock per arc, is now decided by `set_structure()`, since a
  MMCF instance has no sub-Block of its own and the tree is entirely a
  modelling choice; `generate_abstract_variables()` still makes the choice out
  of the Configuration of the Variable if nobody has made it before, and
  throws if the two disagree

### Added 

### Changed 

### Fixed 

## [0.3.1] - 2024-02-28

### Added

- `set_dual()` and `set_potential()` functions

### Changed 

- adapted to new CMake / makefile organisation

### Fixed 

- bunch of minor fixes

## [0.3.0] - 29-06-2022

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

[Unreleased]: https://gitlab.com/smspp/mmcfblock/-/compare/0.3.1...develop
[0.3.1]: https://gitlab.com/smspp/mmcfblock/-/tags/0.3.0...0.3.1
[0.3.0]: https://gitlab.com/smspp/mmcfblock/-/tags/0.2.0...0.3.0
[0.2.0]: https://gitlab.com/smspp/mmcfblock/-/tags/0.1.0...0.2.0
[0.1.0]: https://gitlab.com/smspp/mmcfblock/-/tags/0.1.0
