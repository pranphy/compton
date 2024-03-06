#include "comptonVertex.hh"

#include "G4SystemOfUnits.hh"

#include "G4Material.hh"
#include "G4SystemOfUnits.hh"

comptonVertex::comptonVertex(){
    // Some default material
    fMaterial = NULL;
    fBeamEnergy = 0.0*GeV;
    fRadiationLength = 0.0;
}

comptonVertex::~comptonVertex(){
}
