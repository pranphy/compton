# Data Structure
The simulation saves various branches into the root file. The top level tree is called `T`. The top level structure is like this.

```txt
OOTFile with 2 entries and 30 streamers.
├─ T (TTree)
│  ├─ "units"
│  ├─ "dets.sd"
│  ├─ "dets.lv"
│  ├─ "⋮"
│  ├─ "part"
│  ├─ "hit"
│  ├─ "rate"
│  └─ "sum"
└─ run_data (comptonRunData)
```

The most important branches among these are the "rate", "hit" and "sum" branch.

## Generic Hits

For each event there is a `std::vector<comptonGenericDetectorHit_t>` object with all the hits in all sensitive detectors. So each event has an arry of `hit` object which is of the type `comptonGenericDetectorHit_t`.

The structure of this hit object is

```cpp
// Generic detector hit and sum structure
struct comptonGenericDetectorHit_t {
  int det;
  int id;
  int trid;
  int pid;
  int gen;
  int mtrid;
  double t;
  double x, y, z;
  double xl, yl, zl;
  double r, ph;
  double px, py, pz;
  double pxl, pyl, pzl;
  double sx, sy, sz;
  double p, e, m, k, beta;
  double vx, vy, vz;
  double edep;
  int info;
};

```

So essentially the branches are:

- `hit.id`	Hit identifier
- `hit.det`	Detector identifier: assigned in the geometry with gdml
- `hit.vid`	Volume ID number
- `hit.pid`	Geant4 particle type per http://pdg.lbl.gov/2018/reviews/rpp2018-rev-monte-carlo-numbering.pdf
- `hit.trid`	Geant4 track identifier (1 = first particle created)
- `hit.mtrid`	Geant4 mother track identifier (0 = particle from gun)
- `hit.t`		Hit time [ns]
- `hit.[xyz]`	Hit position, global coordinates [mm]
- `hit.[xyz]l`	Hit position, local coordinates [mm]
- `hit.r`		Hit position radial, global coordinates [mm]
- `hit.ph`	Hit position azimuthal, global coordinates [mm]
- `hit.p`		Particle momentum magnitude [MeV]
- `hit.p[xyz]`	Particle momentum components, lab frame [MeV]
- `hit.s[xyz]`	Particle polarization components, lab frame [1]
- `hit.v[xyz]`	Particle creation vertex position [mm]
- `hit.e`		Particle energy [MeV]
- `hit.m`		Particle rest mass [MeV]
- `hit.edep`	Energy deposited in this detector by this hit [MeV]


## hit Sums

The event level sum of all energy deposition for each detector is also saved in the branch `sum`. The sum branch is again `std::vector<comptonGenericDetectorSum_t>` object, the collection of sum for each sensitive detector. The structure of `comptonGenericDetectorSum_t` is
```cpp
struct comptonGenericDetectorSumByPID_t {
  double x,y,z;
  double edep;
  int pid;
  int n;
};
struct comptonGenericDetectorSum_t {
  std::vector<comptonGenericDetectorSumByPID_t> by_pid;
  double edep;
  int det;
  int vid;
  int n;
};
```

The sum branch also has a substructure of `std::vector<comptonGenericDetectorSumByPID_t>` which just seperates the energy deposition in each sensitive detector by the contributing particle id that deposits that energy.

