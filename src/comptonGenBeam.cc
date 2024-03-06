#include "comptonGenBeam.hh"

#include "comptonEvent.hh"
#include "comptonVertex.hh"
#include "comptonBeamTarget.hh"

#include "G4Material.hh"
#include "G4VPhysicalVolume.hh"
#include "G4LogicalVolume.hh"
#include "G4PhysicalConstants.hh"
#include "G4ParticleTable.hh"

#include "Randomize.hh"
#include "G4SystemOfUnits.hh"
#include "G4PhysicalConstants.hh"

#include "comptontypes.hh"

#include <math.h>

comptonGenBeam::comptonGenBeam()
: comptonVEventGen("beam"),
  fOriginMean(0.0*m,0.0*m,-19.81*m),
  fOriginSpread(0.03,0.03,0.0),
  fOriginModelX(kOriginModelFlat),
  fOriginModelY(kOriginModelFlat),
  fOriginModelZ(kOriginModelFlat),
  fDirection(0.0,0.0,1.0),
  fOriginShift(0.0),
  fIsotropic(false),
  fIsotropicThetaMin(0.0),
  fIsotropicThetaMax(2.0*pi),
  //fCorrelation(0.0653*mrad/mm,0.0653*mrad/mm,0.0),
  fCorrelation(0.0,0.0,0.0),
  fPolarization(0.0,0.0,0.0),
  fRaster(0.0,0.0,0.0),
  fRasterRefZ(0.0),
  fParticleName("e-")
{
    fSamplingType = kNoTargetVolume;
    fApplyMultScatt = true;

    fThisGenMessenger.DeclarePropertyWithUnit("rasterRefZ","mm",fRasterRefZ,"Raster Origin Z: z unit");
    fThisGenMessenger.DeclarePropertyWithUnit("origin","mm",fOriginMean,"origin position mean: x y z unit");
    fThisGenMessenger.DeclareMethodWithUnit("x","mm",&comptonGenBeam::SetOriginXMean,"origin x position mean");
    fThisGenMessenger.DeclareMethodWithUnit("y","mm",&comptonGenBeam::SetOriginYMean,"origin y position mean");
    fThisGenMessenger.DeclareMethodWithUnit("z","mm",&comptonGenBeam::SetOriginZMean,"origin z position mean");

    fThisGenMessenger.DeclarePropertyWithUnit("originspread","mm",fOriginSpread,"origin position spread: x y z unit");
    fThisGenMessenger.DeclareMethodWithUnit("xspread","mm",&comptonGenBeam::SetOriginXSpread,"origin x position spread");
    fThisGenMessenger.DeclareMethodWithUnit("yspread","mm",&comptonGenBeam::SetOriginYSpread,"origin y position spread");
    fThisGenMessenger.DeclareMethodWithUnit("zspread","mm",&comptonGenBeam::SetOriginZSpread,"origin z position spread");
    fThisGenMessenger.DeclareMethod("xmodel",&comptonGenBeam::SetOriginXModel,"origin x position model: flat, gauss");
    fThisGenMessenger.DeclareMethod("ymodel",&comptonGenBeam::SetOriginYModel,"origin y position model: flat, gauss");
    fThisGenMessenger.DeclareMethod("zmodel",&comptonGenBeam::SetOriginZModel,"origin z position model: flat, gauss");

    fThisGenMessenger.DeclareProperty("direction",fDirection,"direction vector (will be normalized): x y z");
    fThisGenMessenger.DeclareMethod("px",&comptonGenBeam::SetDirectionX,"direction x (vector will be normalized before use)");
    fThisGenMessenger.DeclareMethod("py",&comptonGenBeam::SetDirectionY,"direction y (vector will be normalized before use)");
    fThisGenMessenger.DeclareMethod("pz",&comptonGenBeam::SetDirectionZ,"direction z (vector will be normalized before use)");
    fThisGenMessenger.DeclareMethodWithUnit("th","deg",&comptonGenBeam::SetDirectionTh,"direction vector theta angle");
    fThisGenMessenger.DeclareMethodWithUnit("ph","deg",&comptonGenBeam::SetDirectionPh,"direction vector phi angle");
    fThisGenMessenger.DeclareProperty("isotropic",fIsotropic,"direction is isotropic");
    fThisGenMessenger.DeclarePropertyWithUnit("isotropic_theta_min","deg",fIsotropicThetaMin,"minimum theta in isotropic direction");
    fThisGenMessenger.DeclarePropertyWithUnit("isotropic_theta_max","deg",fIsotropicThetaMax,"maximum theta in isotropic direction");

    fThisGenMessenger.DeclarePropertyWithUnit("originshift","mm",fOriginShift,"origin shift along direction vector: s unit");

    fThisGenMessenger.DeclareProperty("polarization",fPolarization,"polarization vector (will be normalized): x y z");
    fThisGenMessenger.DeclareMethod("sx",&comptonGenBeam::SetPolarizationX,"x component of polarization");
    fThisGenMessenger.DeclareMethod("sy",&comptonGenBeam::SetPolarizationY,"y component of polarization");
    fThisGenMessenger.DeclareMethod("sz",&comptonGenBeam::SetPolarizationZ,"z component of polarization");

    fThisGenMessenger.DeclareMethodWithUnit("rasrefz","mm",&comptonGenBeam::SetRasterRefZ,"reference z position where raster is defined");
    fThisGenMessenger.DeclareMethodWithUnit("rasx","mm",&comptonGenBeam::SetRasterX,"raster x spread perpendicular to the beam at z = 0");
    fThisGenMessenger.DeclareMethodWithUnit("rasy","mm",&comptonGenBeam::SetRasterY,"raster y spread perpendicular to the beam at z = 0");
    fThisGenMessenger.DeclareMethod("corrx",&comptonGenBeam::SetCorrelationX,"sensitivity of direction to position in x (in mrad/mm)");
    fThisGenMessenger.DeclareMethod("corry",&comptonGenBeam::SetCorrelationY,"sensitivity of direction to position in y (in mrad/mm)");

    fThisGenMessenger.DeclareMethod("partName",&comptonGenBeam::SetPartName,"name of particle to shoot");
}

comptonGenBeam::~comptonGenBeam() { }

void comptonGenBeam::SetOriginXMean(double x){ fOriginMean.setX(x); }
void comptonGenBeam::SetOriginYMean(double y){ fOriginMean.setY(y); }
void comptonGenBeam::SetOriginZMean(double z){ fOriginMean.setZ(z); }

void comptonGenBeam::SetOriginXSpread(double x){ fOriginSpread.setX(x); }
void comptonGenBeam::SetOriginYSpread(double y){ fOriginSpread.setY(y); }
void comptonGenBeam::SetOriginZSpread(double z){ fOriginSpread.setZ(z); }

comptonGenBeam::EOriginModel comptonGenBeam::GetOriginModelFromString(G4String model) const {
  std::transform(model.begin(), model.end(), model.begin(), ::tolower);
  if (model == "flat")  return kOriginModelFlat;
  if (model == "gauss") return kOriginModelGauss;
  G4cerr << "comptonGenBeam: did not recognize model, assuming flat." << G4endl;
  return kOriginModelFlat;
}
void comptonGenBeam::SetOriginXModel(G4String x){ fOriginModelX = GetOriginModelFromString(x); }
void comptonGenBeam::SetOriginYModel(G4String y){ fOriginModelY = GetOriginModelFromString(y); }
void comptonGenBeam::SetOriginZModel(G4String z){ fOriginModelZ = GetOriginModelFromString(z); }

void comptonGenBeam::SetRasterX(double x){ fRaster.setX(x); }
void comptonGenBeam::SetRasterY(double y){ fRaster.setY(y); }
void comptonGenBeam::SetRasterRefZ(double z){ fRasterRefZ = z; }

void comptonGenBeam::SetDirectionX(double px){ fDirection.setX(px); }
void comptonGenBeam::SetDirectionY(double py){ fDirection.setY(py); }
void comptonGenBeam::SetDirectionZ(double pz){ fDirection.setZ(pz); }
void comptonGenBeam::SetDirectionPh(double ph){ fDirection.setPhi(ph); }
void comptonGenBeam::SetDirectionTh(double th){ fDirection.setTheta(th); }

void comptonGenBeam::SetCorrelationX(double cx){ fCorrelation.setX(cx * mrad/mm); }
void comptonGenBeam::SetCorrelationY(double cy){ fCorrelation.setY(cy * mrad/mm); }

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
    double E = fBeamTarg->fBeamEnergy + m;
    double p = sqrt(E*E - m*m);

    // Start from mean position
    G4ThreeVector origin(fOriginMean);

    // Start from mean direction
    G4ThreeVector direction(fDirection.unit());

    // Add direction range
    if (fIsotropic) {
      double th = acos(G4RandFlat::shoot(cos(fIsotropicThetaMax), cos(fIsotropicThetaMin)));
      double ph = G4RandFlat::shoot(0.0, 2.0*pi);
      direction.setTheta(th);
      direction.setPhi(ph);
    }

    // Add a spread based on chosen model
    //G4ThreeVector spread = GetSpread(fOriginSpread, fOriginModelX, fOriginModelY, fOriginModelZ);
    G4ThreeVector spread = GetSpread(fOriginSpread, kOriginModelGauss, kOriginModelGauss, kOriginModelGauss);

    // Allow for simplistic raster/spreading in beam generator, perpendicular to direction
    G4ThreeVector raster_perpto_direction(fRaster);
    raster_perpto_direction.rotateUz(fDirection.unit());
    G4ThreeVector raster = GetSpread(raster_perpto_direction);

    // Rotate direction for position-angle correlation (maintains unit vector)
    direction.rotateY(+ fCorrelation.x() * raster.x()); // Rotate around Y by X amount
    direction.rotateX(- fCorrelation.y() * raster.y()); // Rotate around X by Y amount

    // Project raster back to origin
    raster.setX(raster.x() + direction.x() * (origin.z() - fRasterRefZ));
    raster.setY(raster.y() + direction.y() * (origin.z() - fRasterRefZ));

    // Add spreads to origin
    origin += raster; // TODO ! ReTurn Raster
    origin += spread;

    // Add shift to origin
    origin += fOriginShift * direction;

    // Override target sampling
    evt->fBeamE = E;
    evt->fBeamMomentum = p * direction;
    evt->fBeamPolarization = fPolarization;
    evt->fVertexPos = origin; // primary vertex

    evt->ProduceNewParticle(
        G4ThreeVector(0.0,0.0,0.0), // relative position to primary vertex
        evt->fBeamMomentum,
        fParticleName,
        evt->fBeamPolarization);

    evt->SetEffCrossSection(1.0);
    evt->SetAsymmetry(0.0);

    evt->SetQ2(0.0);
    evt->SetW2(0.0);
}
