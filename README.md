# MMCFBlock

(So far, only a rough sketch of) a Block for Multicommodity Min-Cost Flow
problems (MMCF).

The rationale for the class is that MMCF can be read in a variety of
formats and can be represented in a number of different ways. The
MMCFBlock so far basically only provides a convenient way to read an
instance out of the many formats, such as some of those available from

http://groups.di.unipi.it/optimize/Data/MMCF.html

and construct some formulations, i.e.:

- the standard flow formulation in which k MCFBlock sub-Block are
  constructed, one for each commodity, and the linking constraints
  are handled in the father MMCFBlock;

- [other ones to follow].

MMCFBlock is still in very early development.

## Getting started

These instructions will let you build MMCFBlock on your system.

### Requirements

- [SMS++ core library](https://gitlab.com/smspp/smspp)
- [MCFBlock](https://gitlab.com/smspp/mcfblock)

### Build and install with CMake

Configure and build the library with:

```sh
mkdir build
cd build
cmake ..
make
```

The library has the same configuration options of
[SMS++](https://gitlab.com/smspp/smspp/wikis/custom).
Optionally, install the library in the system with:

```sh
sudo make install
```

### Usage with CMake

After the module is built, you can use it in your CMake project with:
```cmake
find_package(MMCFBlock)
target_link_libraries(<my_target> SMS++::MMCFBlock)
```

## Contributing

Please read [CONTRIBUTING.md](CONTRIBUTING.md) for details on our code of
conduct, and the process for submitting merge requests to us.

## Authors

- **Antonio Frangioni**  
  *Operations Research Group*  
  Dipartimento di Informatica  
  Università di Pisa

- **Enrico Gorgone**  
  Dipartimento di Matematica ed Informatica  
  Università di Cagliari

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
