# MMCFBlock

(So far, only a rough sketch of) a `Block` for Multicommodity Min-Cost
Flow (MMCF) and Multicommodity Network Design problems.

The rationale for the class is that MMCF can be read in a variety of
formats and can be represented in a number of different ways. The
`MMCFBlock` so far basically only provides a convenient way to read an
instance out of the many formats, such as some of those available from

    https://commalab.di.unipi.it/datasets/mmcf/

and construct the two most basic formulations, i.e.:

- the standard *flow formulation* in which *k* (number of commodities)
  `MCFBlock` sub-`Block` are constructed, one for each commodity, and
  the linking constraints (mutual capacity, with possibly strong forcing
  constraints in the network design case) are handled in the father
  `MMCFBlock`;

- the standard *knapsack formulation* in which *m* (number of arcs)
  `BinaryKnapsackBlock` sub-`Block` are constructed, one for each arc,
  and the linking constraints (flow conservation ones) are handled in
  the father `MMCFBlock`.

Other formulations will perhaps follow. `MMCFBlock` is still in very early
stages of the development.


## Getting started

These instructions will let you build `MMCFBlock` on your system.

### Requirements

- [SMS++ core library](https://gitlab.com/smspp/smspp)

- [MCFBlock](https://gitlab.com/smspp/mcfblock)

- [BinaryKnapsackBlock](https://gitlab.com/smspp/binaryknapsackblock)

### Build and install with CMake

Configure and build the library with:

```sh
mkdir build
cd build
cmake ..
cmake --build .
```

The library has the same configuration options of
[SMS++](https://gitlab.com/smspp/smspp-project/-/wikis/Customize-the-configuration).
Optionally, install the library in the system with:

```sh
cmake --install .
```

### Usage with CMake

After the module is built, you can use it in your CMake project with:

```cmake
find_package(MMCFBlock)
target_link_libraries(<my_target> SMS++::MMCFBlock)
```

### Build and install with makefiles

Carefully hand-crafted makefiles have also been developed for those unwilling
to use CMake. Makefiles build the executable in-source (in the same directory
tree where the code is) as opposed to out-of-source (in the copy of the
directory tree constructed in the build/ folder) and therefore it is more
convenient when having to recompile often, such as when developing/debugging
a new module, as opposed to the compile-and-forget usage envisioned by CMake.

Each executable using `MMCFBlock` has to include a "main makefile" of the
module, which typically is either [makefile-c](makefile-c) including all
necessary libraries comprised the "core SMS++" one, or
[makefile-s](makefile-s) including all necessary libraries but not the "core
SMS++" one (for the common case in which this is used together with other
modules that already include them). One relevant case is the
[tester comparing MMCFBlock with the old MMCFClass](https://gitlab.com/smspp/tests/-/tree/develop/MMCFBlock?ref_type=heads).
The makefiles in turn recursively include all the required other makefiles,
hence one should only need to edit the "main makefile" for compilation type
(C++ compiler and its options) and it all should be good to go. In case some
of the external libraries are not at their default location, it should only be
necessary to create the `../extlib/makefile-paths` out of the
`extlib/makefile-default-paths-*` for your OS `*` and edit the relevant bits
(commenting out all the rest).

Check the [SMS++ installation wiki](https://gitlab.com/smspp/smspp-project/-/wikis/Customize-the-configuration#location-of-required-libraries)
for further details.


## Getting help

If you need support, you want to submit bugs or propose a new feature, you can
[open a new issue](https://gitlab.com/smspp/mmcfblock/-/issues/new).


## Contributing

Please read [CONTRIBUTING.md](CONTRIBUTING.md) for details on our code of
conduct, and the process for submitting merge requests to us.


## Authors

- **Enrico Gorgone**  
  Dipartimento di Matematica e Informatica  
  Università di Cagliari

- **Francesco Demelas**  
  Laboratoire d'Informatique de Paris Nord  
  Universite' Sorbonne Paris Nord

### Contributors

- **Antonio Frangioni**  
  Dipartimento di Informatica  
  Università di Pisa


## License

This code is provided free of charge under the [GNU Lesser General Public
License version 3.0](https://opensource.org/licenses/lgpl-3.0.html) -
see the [LICENSE](LICENSE) file for details.


## Disclaimer

The code is currently provided free of charge under an open-source license.
As such, it is provided "*as is*", without any explicit or implicit warranty
that it will properly behave or it will suit your needs. The Authors of
the code cannot be considered liable, either directly or indirectly, for
any damage or loss that anybody could suffer for having used it. More
details about the non-warranty attached to this code are available in the
license description file.
