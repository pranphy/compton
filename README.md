# Simulations for the Compton Polarimeter at Jefferson Lab
## Community

Simulations are coordinated on the [12gevmoller_sim@jlab.org](https://mailman.jlab.org/mailman/listinfo/12gevmoller_sim) mailing list and the [JLab 12 GeV Slack workspace](https://jlab12gev.slack.com) (in particular, the `#moller_simulation` channel). Anyone with a jlab.org email address can join without invitation. Feel free to contact developers there with questions.

Simulations can be run in interactive mode when not specifying arguments, or in batch mode when specifying a macro:
```
Usage:
 compton [-g geometry] [-m macro] [-u session] [-r seed] [-t nthreads] [macro]
```

## Analyzing the output

You can access the output file with a regular root installation. A listing of the [output variables](README.variables.md) is available for reference.

To take advantage of dedicated functionality for the data types in the output file, you will need to follow the more detailed [analysis instructions](analysis/README.md). To simplify this, you can also use the helper executable `compoot` which is available wherever `compton` is available.

