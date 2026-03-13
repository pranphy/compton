#include "comptonGenBeam.hh"

#include "comptonEvent.hh"
#include "comptonVertex.hh"

#include "G4PhysicalConstants.hh"
#include "G4ParticleTable.hh"

#include "Randomize.hh"
#include "G4SystemOfUnits.hh"
#include "G4PhysicalConstants.hh"

#include "comptontypes.hh"

#include <math.h>

comptonGenBeam::comptonGenBeam()
: comptonVEventGen("beam"),
  fOriginMean(0.0*m,0.0*m,0.0*m),
  fOriginSpread(0.03,0.03,0.0),
  fOriginModelX(kOriginModelGauss),
  fOriginModelY(kOriginModelGauss),
  fOriginModelZ(kOriginModelGauss),
  fDirection(0.0,0.0,1.0),
  fIsotropic(false),
  fCorrelation(0.0,0.0,0.0),
  fPolarization(0.0,0.0,0.0),
  fBeamCurrent(0.0),
  fBeamEnergy(0.0),
  fParticleName("e-")
{

    fThisGenMessenger.DeclarePropertyWithUnit("beamcurr","microampere",fBeamCurrent,"Beam current");
    fThisGenMessenger.DeclarePropertyWithUnit("beamene","GeV",fBeamEnergy,"Beam energy");
    fThisGenMessenger.DeclarePropertyWithUnit("origin","mm",fOriginMean,"origin position mean: x y z unit");
    fThisGenMessenger.DeclarePropertyWithUnit("originspread","mm",fOriginSpread,"origin position spread: x y z unit");
    fThisGenMessenger.DeclareProperty("direction",fDirection,"direction vector (will be normalized): x y z");

    fThisGenMessenger.DeclareProperty("polarization",fPolarization,"polarization vector (will be normalized): x y z");
    fThisGenMessenger.DeclareMethod("sx",&comptonGenBeam::SetPolarizationX,"x component of polarization");
    fThisGenMessenger.DeclareMethod("sy",&comptonGenBeam::SetPolarizationY,"y component of polarization");
    fThisGenMessenger.DeclareMethod("sz",&comptonGenBeam::SetPolarizationZ,"z component of polarization");

    fThisGenMessenger.DeclareMethod("partName",&comptonGenBeam::SetPartName,"name of particle to shoot");
}

comptonGenBeam::~comptonGenBeam() { }


comptonGenBeam::EOriginModel comptonGenBeam::GetOriginModelFromString(G4String model) const {
  std::transform(model.begin(), model.end(), model.begin(), ::tolower);
  if (model == "flat")  return kOriginModelFlat;
  if (model == "gauss") return kOriginModelGauss;
  G4cerr << "comptonGenBeam: did not recognize model, assuming flat." << G4endl;
  return kOriginModelFlat;
}

void comptonGenBeam::SetPolarizationX(double sx){ fPolarization.setX(sx); }
void comptonGenBeam::SetPolarizationY(double sy){ fPolarization.setY(sy); }
void comptonGenBeam::SetPolarizationZ(double sz){ fPolarization.setZ(sz); }

void comptonGenBeam::SetPartName(G4String& name){
  G4ParticleTable* particleTable = G4ParticleTable::GetParticleTable();
  G4ParticleDefinition* particle = particleTable->FindParticle(name);
  if (particle != nullptr) fParticleName = name;
  else {
    G4cerr << "comptonGenBeam: particle " << name << " not recognized." << G4endl;
    exit(-1);
  }
}

G4double comptonGenBeam::GetSpread(G4double spread, EOriginModel model)
{
  if (model == kOriginModelFlat)
    return G4RandFlat::shoot(-spread/2.0, +spread/2.0);
  if (model == kOriginModelGauss)
    return G4RandGauss::shoot(0.0, spread);
  else return 0.0;
}

G4ThreeVector comptonGenBeam::GetSpread(G4ThreeVector spread,
  EOriginModel x, EOriginModel y, EOriginModel z)
{
  G4ThreeVector sample(0.0,0.0,0.0);
  sample.setX(GetSpread(spread.x(), x));
  sample.setY(GetSpread(spread.y(), y));
  sample.setZ(GetSpread(spread.z(), z));
  return sample;
}

void comptonGenBeam::SamplePhysics(comptonVertex * /*vert*/, comptonEvent *evt)
{
    G4ParticleTable* particleTable = G4ParticleTable::GetParticleTable();
    G4ParticleDefinition* particle = particleTable->FindParticle(fParticleName);

    // Get initial beam energy instead of using other sampling
    double m = particle->GetPDGMass();
    double E = fBeamEnergy + m;
    double p = sqrt(E*E - m*m);

    // Start from mean position
    G4ThreeVector origin(fOriginMean);

    // Start from mean direction
    G4ThreeVector direction(fDirection.unit());

    G4ThreeVector spread = GetSpread(fOriginSpread, kOriginModelGauss, kOriginModelGauss, kOriginModelGauss);

    origin += spread;

    // Override target sampling
    evt->fBeamE = E;
    evt->fBeamMomentum = p * direction;
    evt->fBeamPolarization = fPolarization;
    evt->fVertexPos = origin; // primary vertex

    evt->ProduceNewParticle(
        origin,
        evt->fBeamMomentum,
        fParticleName);

    evt->SetEffCrossSection(1.0);

    double rate = fBeamCurrent/(e_SI*coulomb);
    evt->SetRate(rate);

}
