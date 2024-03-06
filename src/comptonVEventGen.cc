#include "comptonVEventGen.hh"

#include <cassert>

#include "G4ParticleGun.hh"
#include "G4RotationMatrix.hh"

#include "comptonBeamTarget.hh"
#include "comptonVertex.hh"
#include "comptonEvent.hh"
#include "comptonRun.hh"
#include "comptonRunData.hh"

G4double comptonVEventGen::fTh_min = 0.0*deg;
G4double comptonVEventGen::fTh_max = 180.0*deg;
G4double comptonVEventGen::fThCoM_min = 0.0*deg;
G4double comptonVEventGen::fThCoM_max = 180.0*deg;
G4double comptonVEventGen::fPh_min = 0.0*deg;
G4double comptonVEventGen::fPh_max = 360.0*deg;
G4double comptonVEventGen::fE_min = 0.0*deg;
G4double comptonVEventGen::fE_max = 11.0*GeV;

comptonVEventGen::comptonVEventGen(const G4String name)
: fName(name),
  fBeamPol("0"),
  fNumberOfParticles(1),fParticleGun(0),
  fBeamTarg(0),
  fThisGenMessenger(this,"/compton/evgen/" + name + "/","Remoll " + name + " generator properties")
{
    // Set initial number of particles and create particle gun
    SetNumberOfParticles(fNumberOfParticles);

    // Create event generator messenger
    fEvGenMessenger.DeclarePropertyWithUnit("emax","GeV",fE_max,"Maximum generation energy");
    fEvGenMessenger.DeclarePropertyWithUnit("emin","GeV",fE_min,"Minimum generation energy");
    fEvGenMessenger.DeclarePropertyWithUnit("thmax","deg",fTh_max,"Maximum generation theta angle");
    fEvGenMessenger.DeclarePropertyWithUnit("thmin","deg",fTh_min,"Minimum generation theta angle");
    fEvGenMessenger.DeclarePropertyWithUnit("phmax","deg",fPh_max,"Maximum generation phi angle");
    fEvGenMessenger.DeclarePropertyWithUnit("phmin","deg",fPh_min,"Minimum generation phi angle");
    fEvGenMessenger.DeclarePropertyWithUnit("thcommax","deg",fThCoM_max,"Maximum CoM generation theta angle");
    fEvGenMessenger.DeclarePropertyWithUnit("thcommin","deg",fThCoM_min,"Minimum CoM generation theta angle");
    fEvGenMessenger.DeclareProperty("beamPolarization",fBeamPol,"Polarization direction: +L, +H, +V, -L, -H, -V, 0");
    fEvGenMessenger.DeclareMethod(
        "printlimits",
        &comptonVEventGen::PrintEventGen,
        "Print the event generator limits");

    fSamplingType = kActiveTargetVolume;
    fApplyMultScatt = false;
}

comptonVEventGen::~comptonVEventGen()
{
}

void comptonVEventGen::PrintEventGen()
{
  G4cout << "Event generator: " << fName << G4endl;
  G4cout << "E =     [" << fE_min/GeV  << "," << fE_max/GeV  << "] GeV" << G4endl;
  G4cout << "phi =   [" << fPh_min/deg << "," << fPh_max/deg << "] deg" << G4endl;
  G4cout << "theta = [" << fTh_min/deg << "," << fTh_max/deg << "] deg" << G4endl;
  G4cout << "theta (COM) = [" << fThCoM_min/deg << "," << fThCoM_max/deg << "] deg" << G4endl;
}

void comptonVEventGen::SetNumberOfParticles(G4int n)
{
  // Store new number of particles
  fNumberOfParticles = n;

  // Delete old particle gun
  if (fParticleGun != nullptr) {
    delete fParticleGun;
    fParticleGun = 0;
  }
  // Create new particle gun
  fParticleGun = new G4ParticleGun(fNumberOfParticles);
}

comptonEvent* comptonVEventGen::GenerateEvent()
{
    // Set up beam/target vertex
    comptonVertex vert   = fBeamTarg->SampleVertex(fSamplingType);

    /////////////////////////////////////////////////////////////////////
    // Create and initialize values for event
    comptonEvent *thisev = new comptonEvent();
    thisev->SetBeamTarget(fBeamTarg);

    thisev->fVertexPos    = fBeamTarg->fVer;
    if( fApplyMultScatt ) {
        thisev->fBeamMomentum = fBeamTarg->fSampledEnergy*(fBeamTarg->fDir.unit());
    } else {
        thisev->fBeamMomentum = fBeamTarg->fSampledEnergy*G4ThreeVector(0.0, 0.0, 1.0);
    }
    /////////////////////////////////////////////////////////////////////

    SamplePhysics(&vert, thisev);

    PolishEvent(thisev);

    return thisev;
}


void comptonVEventGen::PolishEvent(comptonEvent *ev) {
    /*!
       Here it's our job to:
          Make sure the event is sane
          Apply multiple scattering effects to the final
        products if applicable
      Calculate rates from our given luminosity
      Calculate measured asymmetry from polarization
      Calculate vertex offsets
     */

    if( !ev->EventIsSane() ) {
        G4cerr << __FILE__ << " line " << __LINE__ << ":  Event check failed for generator " << fName << ".  Aborting" << G4endl;
        ev->Print();
        exit(1);
    }

    G4ThreeVector rotax      = (-1)*(fBeamTarg->fDir.cross(G4ThreeVector(0.0, 0.0, 1.0))).unit();
    G4RotationMatrix msrot;
    msrot.rotate(fBeamTarg->fDir.theta(), rotax);

    std::vector<G4ThreeVector>::iterator iter;

    if( fApplyMultScatt ) {
        for( iter = ev->fPartRealMom.begin(); iter != ev->fPartRealMom.end(); iter++ ) {
            //  rotate direction vectors based on multiple scattering
            (*iter) *= msrot;
        }

        // Rotate position offsets due to multiple scattering
        for( iter = ev->fPartPos.begin(); iter != ev->fPartPos.end(); iter++ ) {
            //  rotate direction vectors based on multiple scattering
            (*iter) *= msrot;
        }
    }

    // Add base vertex
    for( iter = ev->fPartPos.begin(); iter != ev->fPartPos.end(); iter++ ) {
        (*iter) += ev->fVertexPos;
    }

    ev->fmAsym = ev->fAsym*fBeamTarg->fBeamPolarization;
}
