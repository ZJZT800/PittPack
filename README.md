# PittPack
PittPack is an Open-Source Poisson’s Equation Solver for Extreme-Scale Computing with Accelerators <br/>
It is developed as part of the GEM3D project funded by NSF at University of Pittsburgh, PA, USA. 

## Configuration 
Automatically configures the code for Stampede2 (https://www.tacc.utexas.edu/systems/stampede2), Comet (https://www.sdsc.edu/services/hpc/hpc_systems.html) and Bridges (https://www.psc.edu/resources/computing/bridges) clusters 

## Linux 
```
source config.sh 
```
When prompted respond by entering 0 or 1    
* (0) : will configure PittPack for CPU clusters 
* (1) : will configure PittPack for GPU clusters



## Installation
PittPack requires the following libraries
  * cmake 
  * MPI 
  * FFTW3 
  * PGI  
  * HDF5 with Parallel IO


##  Build  
PittPack uses CMakeLists.txt and CMakeModules folder to detect the library paths. <br/>
These two components are crucial for complilation of PittPack.
Perform the following steps
```
  cd build
  cmake ..
  make -j 4
```
the executable will be placed in /bin


## Run
mpirun -np NumberofProcs ./bin/PittPack nx ny nz 
  * nx: Number of elements in X-direction
  * ny: Number of elements in Y-direction
  * nz: Number of elements in Z-direction
 
## Directory
```
PittPack
│   README.md
│   CmakeLists.txt    
│   LICENSE
│   config.sh
│
└─── src
│   │   file011.txt
│   │   file012.txt
│   │   
│   │
│   │
│   └─── include
│       │   file111.txt
│       │   file112.txt
│       │   ...
│   
└─── bin
│       PittPack (executable)  
│  
│
└───   


 
```

## Detailed usage
For complete documentation visit www.pittpack.com
