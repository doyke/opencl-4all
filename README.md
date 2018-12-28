# OpenCL-forró (opencl-4all)

Collection of OpenCL kernels for GPU/FPGA execution

## Author

André Bannwart Perina

## Introduction

***SORRY FOR THE MESS, WE ARE STILL IN CONSTRUCTION :)***

This is an extensive set of OpenCL applications organised for execution and profiling on GPUs and FPGAs. Focused on analysing the FPGA performance and energy consumption when compared against GPUs, this repository is comprised of 3 experiments, each having a different level of OpenCL FPGA-biased optimisations. Comparison results are also available.

## OpenCL kernels

We collected kernels from different repositories:
* Rodinia benchmark:
	* https://rodinia.cs.virginia.edu/doku.php
* SHOC benchmark:
	* https://github.com/vetter/shoc
* Personal kernels, adapted from:
	* https://github.com/freecores/bluespec-reedsolomon
* WELLER, Dennis et al. Energy efficient scientific computing on fpgas using opencl. In: Proceedings of the 2017 ACM/SIGDA International Symposium on Field-Programmable Gate Arrays. ACM, 2017. p. 247-256:
	* https://github.com/mediroozmeh/FPGA_BitonicSorting
* MUSLIM, Fahad Bin et al. Efficient FPGA implementation of OpenCL high-performance computing applications via high-level synthesis. IEEE Access, v. 5, p. 2747-2762, 2017:
	* http://cdnc.itec.kit.edu/OpenCLFPGA.php

## Used hardware accelerators

We used the following hardware for our executions and profiling:

* ***FPGA 1:*** BittWare S5PH-Q (Intel FPGA Stratix V 5sgxa7);
	* Intel FPGA SDK for OpenCL version 16.1;
* ***FPGA 2:*** Intel FPGA Hardware Accelerator Program (Intel FPGA Arria 10);
* ***GPU 1:*** NVIDIA Quadro (NVIDIA Quadro K620);
	* NVIDIA OpenCL SDK 384.81;
* ***GPU 2:*** EVGA ACX 2.0 (NVIDIA GeForce GTX980).
	* NVIDIA OpenCL SDK 384.81.

To use different platforms, modifications may be necessary to the projects makefiles, such as (not limited to):

* Changing `GPUFLAGS` if other brand of GPU is used;
* Changing `aoc ... --board <boardname> ...` accordingly if other platform containing Intel FPGA is used;
* Changing `AOCLFLAGS` and compilation commands if other brand of FPGA is used.

## Repository structure

This repository contains three root folders, one for each experiment:

* Experiment A: OpenCL kernels with no FPGA optimisations (NDRange);
* Experiment B: OpenCL kernels with FPGA optimisations but without change in execution model (NDRange);
* Experiment C: OpenCL kernels with FPGA optimisations, changing also execution model (task).

### Experiment structure

Each experiment has a description spreadsheet, several helper scripts, result files and OpenCL projects prepared for GPU compilation and FPGA synthesis.

#### Description spreadsheet

The experiment description spreadsheet (`description.ods`) contains information about the OpenCL applications including: application original source, data set, modifications and compilation report.

#### Helper scripts

There are several BASH helper scripts, useful for acquiring information or performing tasks automatically for all projects:

* Check which projects are compiled (including host executable and kernel object file);
* Collect operational frequency of all kernels (FPGA);
* Calculate checksum of all synthesised kernels (FPGA);
* Compile host executables (GPU);
* Run kernels.

To run the first script:
```
$ cd path/to/experiment
$ ./projectschecker.sh
```

To run all other scripts:
* Experiment A:
	```
	$ cd path/to/experiment/Unop_sv
	$ ../script.sh
	```
* Experiment B:
	```
	$ cd path/to/experiment/Opts_sv/2_Full
	$ ../../script.sh
	```
* Experiment C:
	```
	$ cd path/to/experiment/Opts_sv/3_FullTask
	$ ../../script.sh
	```

#### Result files

The execution results are divided in two types:

* Raw: ```csv``` files auto-generated with execution times of all kernels;
* Interpreted: spreadsheets containing execution times, several analyses and why failing kernels did not execute.

### OpenCL projects

Inside `Unop_sv`, `Opts_sv/2_Full` and `Opts_sv/3_FullTask` for experiments A, B and C respectively, there are several OpenCL projects each with a single kernel. The host code for all kernels were generated by a tool called ***hostcodegen*** (https://github.com/comododragon/hostcodegen), where a XML description file is used to create a host code with failure tests, profiling and results check. All projects have the same structure, with the following standard files:

* `src/kern.cl`: OpenCL kernel;
* `src/kern.fpga.xml`: OpenCL XML description of kernel used by `hostcodegen` (FPGA version);
* `src/kern.gpu.xml`: OpenCL XML description of kernel used by `hostcodegen` (GPU version);
* `src/host.fpga.c`: OpenCL host source code (FPGA version);
* `src/host.gpu.c`: OpenCL host source code (GPU version);
* `include/common.h`: Common header used for nice report printing;
* `include/prepostambles.h`: Header containing custom logic for populating the input data and expected output results;
* `Makefile`: a Makefile (lol).

Using the provided `Makefile`, it is possible to compile for GPU, synthesise for FPGA or emulate for FPGA.

*** We used NVIDIA and Intel FPGA boards. To adapt for your hardware, modifications may be necessary in the makefiles. Please see Section "Used hardware accelerators" for information about which platforms we used.***

To compile and execute for GPU:
```
$ make gpu/execute
$ cd gpu
$ ./execute
```

To compile and execute for FPGA (brace yourselves for long compilation):
```
$ make fpga/bin/execute
$ cd fpga/bin
$ ./execute
```

To emulate FPGA execution:
```
$ make fpga/emu/emulate
$ cd fpga/emu
$ env CL_CONTEXT_EMULATOR_DEVICE_ALTERA=1 ./emulate
```

## Licence

For the repository licence, see LICENSE file.

Some files are licensed under different terms. Please refer to the file's header for further information.
