#include "comptonGenLaser.hh"

#include "comptonEvent.hh"
#include "comptonVertex.hh"
#include "G4SystemOfUnits.hh"

#include "comptontypes.hh"

#include <math.h>

comptonGenLaser::comptonGenLaser()
: comptonVEventGen("laser"),
   fMaxPhotonEnergy(100*CLHEP::eV),
   fLaserWavelength(532*nm),
   fOriginMean(0.0,210.49*mm,0.0)
{
    fLaserWavelength = CLHEP::h_Planck * CLHEP::c_light / fMaxPhotonEnergy;

    fThisGenMessenger.DeclarePropertyWithUnit("origin","mm",fOriginMean,"origin position mean: x y z unit");
    fThisGenMessenger.DeclarePropertyWithUnit("energy","MeV",fMaxPhotonEnergy,"Energy of photon");

}

comptonGenLaser::~comptonGenLaser() { }


void comptonGenLaser::SamplePhysics(comptonVertex * /*vert*/, comptonEvent *evt)
{

  const G4ThreeVector gamma_direction{0,0,1};
  const double gamma_E = fMaxPhotonEnergy;
  const G4ThreeVector gamma_momentum = gamma_E*gamma_direction;
  //std::printf("+++ The position is %lf %lf %lf\n", fOriginMean.x(), fOriginMean.y(), fOriginMean.z());

  evt->ProduceNewParticle( G4ThreeVector(0.0,210.49, 0.0), gamma_momentum, "gamma");

  G4double APV = 0.001;

  evt->SetAsymmetry(APV);
  evt->SetThCoM(0.0001);
  evt->SetEffCrossSection(0.001);

  //evt->SetQ2( 2.0*e_com*e_com*(1.0-cos(thcom)) );
  // Q2 is not actually well defined
  //evt->SetQ2( 0.0 );

}
