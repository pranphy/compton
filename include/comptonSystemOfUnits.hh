#ifndef __COMPTONSYSTEMOFUNITS_HH
#define __COMPTONSYSTEMOFUNITS_HH

#include "G4Types.hh"
#include "G4SystemOfUnits.hh"

// New units in the compton namespace
namespace compton {
  static const double ppm = 1E-6;
  static const double ppb = 1E-9;
}

// Allow for easy use without specifying namespace
using compton::ppm;
using compton::ppb;

#endif // __COMPTONSYSTEMOFUNITS_HH
