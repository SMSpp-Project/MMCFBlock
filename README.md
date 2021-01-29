# mmcfblock

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
