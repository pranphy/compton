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
cmake ..
make -j$(nproc)
```

This should produce `compton` executable which can be used to run simulations as well as `compoot` executable which can be used to analyze the output.

## Community

Simulations are coordinated on the [12gevmoller_sim@jlab.org](https://mailman.jlab.org/mailman/listinfo/12gevmoller_sim) mailing list and the [JLab 12 GeV Slack workspace](https://jlab12gev.slack.com) (in particular, the `#moller_simulation` channel). Anyone with a jlab.org email address can join without invitation. Feel free to contact developers there with questions.

Simulations can be run in interactive mode when not specifying arguments, or in batch mode when specifying a macro:
```
Usage:
 compton [-g geometry] [-m macro] [-u session] [-r seed] [-t nthreads] [macro]
```

## Analyzing the output

You can access the output file with a regular root installation. A listing of the [output variables](README.variables.md) is available for reference.

