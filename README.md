# mmcfblock

MMCF (multicommodity min cost flow) problem using MILP Solver, The class reads in input an instances according to different formats. Then, the problem is constructed. 

To compile is required to have at same folder level SMS++ and MCFblock The tester is given in the "tests" project. 

The mmcfblock tester contains:

1. the main file: main.ccp;
2. the generator of the instaces. To generate the istamces run the script file "genbatch"; 
3. the instances (folder called data). That foledr is empty at beginning and it will be filled as you run "genbatch'. Thress kinds of instacmes are consedered: the Canad, the Mnetgen and the JLF. ;
4. the batch files (folder called batch). To launch the tests run the scipt file called "auto";
5. the makefile.

To compile the test file run the command make, but before is required to have installed at the same level of the tests foleder project and MILPSolver. Af for MMCFClass., The test project comes out along with MMCFClass. in fact, the test makes a comparison between MMCFCplex (under the interface MMCFClass) and the MILPSolver (written for SMS++).


