#include "comptonGenHyperon.hh"

// Geant4 includes
#include "G4Material.hh"

// compton includes
#include "comptonBeamTarget.hh"
#include "comptonEvent.hh"
#include "comptonVertex.hh"
#include "comptontypes.hh"
#include "comptonRun.hh"

// System includes
#include <iostream>
#include <sstream>
#include <iterator>
#include <string>

#include "G4AutoLock.hh"
namespace { G4Mutex comptonGenHyperonMutex = G4MUTEX_INITIALIZER; }

comptonFileReader* comptonGenHyperon::fFileReader = 0;

comptonGenHyperon::comptonGenHyperon()
: comptonVEventGen("hyperon"),
  fDebugLevel(0),
  fFile("generators/aniol/hyperon_outp.dat"),fParticle("lambda"),
  fRUnit(cm),fPUnit(GeV),fWUnit(barn)
{
  // Add to generic messenger
  fThisGenMessenger.DeclareProperty("debug",fDebugLevel,"Debug level");
  fThisGenMessenger.DeclareProperty("file",fFile,"Input filename");
  fThisGenMessenger.DeclareProperty("skip",fSkip,"Number of lines to skip");
  fThisGenMessenger.DeclareProperty("particle",fParticle,"Particle name");
  fThisGenMessenger.DeclarePropertyWithUnit("runit","cm",fRUnit,"Units of position");
  fThisGenMessenger.DeclarePropertyWithUnit("punit","GeV",fPUnit,"Units of momentum");
  fThisGenMessenger.DeclarePropertyWithUnit("wunit","barn",fWUnit,"Units of weight");
}

comptonGenHyperon::~comptonGenHyperon()
{
  G4AutoLock lock(&comptonGenHyperonMutex);
  if (fFileReader != nullptr) { delete fFileReader; fFileReader = 0; }
}

comptonFileReader* comptonGenHyperon::GetFileReader() const
{
  G4AutoLock lock(&comptonGenHyperonMutex);
  if (fFileReader == nullptr) {
    fFileReader = new comptonFileReader(fFile,fSkip,fDebugLevel);
  }
  return fFileReader;
}

void comptonGenHyperon::SamplePhysics(comptonVertex* /*vert*/, comptonEvent* evt)
{
  comptonFileEvent event;

  // Limit scope of mutex to read from buffered file
  if (GetFileReader() != nullptr) {
    G4AutoLock lock(&comptonGenHyperonMutex);
    event = fFileReader->GetAnEvent(); // don't use GetFileReader, race condition
  }

  // Create event
  evt->SetRate(event.w() / fWUnit);
  evt->SetAsymmetry(0.0);
  evt->SetEffCrossSection(1.0);
  evt->ProduceNewParticle(event.r() / fRUnit, event.p() / fPUnit, fParticle);
}
