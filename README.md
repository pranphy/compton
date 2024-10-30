# Simulations for the Compton Polarimeter at Jefferson Lab Hall A
## Compilation
This uses cmake build system to compile. Make sure that you have the latest version of cmake.
clone the directory
```bash
git clone https://github.com/pranphy/compton.git
```

Compilation is just as simple as mkaing a build directory and running cmake.
```bash
cd compton
mkdir build
cd build
cmake ..
make -j$(nproc)
```

This should produce `compton` executable which can be used to run simulations as well as `compoot` executable which can be used to analyze the output.


## Running simulations

Simulations can be run in interactive mode when not specifying arguments, or in batch mode when specifying a macro:
```
Usage:
 compton [-g geometry] [-m macro] [-u session] [-r seed] [-t nthreads] [macro]
```

Run simulations with `compton` executable:
```bash
./build/compton
```
This will open up a GUI with the default geometry. Without argument it will start with a macro file `macros/runexample_vis.mac` which sets up the GUI and renders the default geometry.


## Batch mode

To run simulation in batch mode you can pass a macro file as an argument:
```bash
./build/compton macros/test.mac
```

## Analyzing the output

You can access the output file with a regular root installation. A listing of the [output variables](README.variables.md) is available for reference.

