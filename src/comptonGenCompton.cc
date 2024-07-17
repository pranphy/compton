#include "comptonGenCompton.hh"

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

#include <fstream>
#include <math.h>

comptonGenCompton::comptonGenCompton()
: comptonVEventGen("compton"),
   fMaxPhotonEnergy(0.0),
   fElectronEnergy(11*GeV),
   fLaserWavelength(532*nm)
{
    fSamplingType = kNoTargetVolume;
    auto me = CLHEP::electron_mass_c2;
    auto r0 = CLHEP::classic_electr_radius;
    G4double gma =  fElectronEnergy/CLHEP::electron_mass_c2;

    fLaserEnergy = CLHEP::h_Planck * CLHEP::c_light / fLaserWavelength;
    fAParameter = 1 / (1 + (4*fLaserEnergy*fElectronEnergy)/ (me*me));
    //fMaxPhotonEnergy = fElectronEnergy * (1 - fAParameter);
    fMaxPhotonEnergy = 4*fAParameter*fLaserEnergy*(gma*gma);


    G4double am1 = fAParameter - 1.0;
    for(int i = 0; i < 10000; i++) {
        G4double rho = G4double(i/10000.0);
        G4double term1 = rho*rho*am1*am1/(1.+rho*am1);
        G4double term3 = (1.-rho*(1.0+fAParameter))/(1.+rho*am1);
        fCXdSig_dRho[i] = 2 * CLHEP::pi * r0*r0 * fAParameter *(term1 +1.0 + term3*term3);
        //of<<fCXdSig_dRho[i]<<std::endl;
    }
}

comptonGenCompton::~comptonGenCompton() { }



G4double comptonGenCompton::GetRandomRho()
{
  G4RandGeneral GenDist(fCXdSig_dRho,10000);
  return GenDist.shoot();
}


double comptonGenCompton::get_max_k(){
    double k = fLaserEnergy;
    double E = fElectronEnergy;
    double m = CLHEP::electron_mass_c2;
    double p = std::sqrt(E*E - m*m);
    double alpha_c = 20*1e-3*CLHEP::radian;
    double theta_gamma = alpha_c;

    double k_max = k*( ( E + p*cos(alpha_c) ) / ( E + k - p * cos(theta_gamma)  + k ) );
    return k_max;
}

void comptonGenCompton::SamplePhysics(comptonVertex * /*vert*/, comptonEvent *evt)
{

  double rho = GetRandomRho();
  double gamma_E = rho * fMaxPhotonEnergy;
  G4ThreeVector gamma_direction;
  G4ThreeVector direction_e;
  G4double gma =  fElectronEnergy/CLHEP::electron_mass_c2;
  double gamma_theta = std::sqrt( 4 * fLaserEnergy/gamma_E - 1/( fAParameter*gma*gma));
  double gamma_phi = CLHEP::RandFlat::shoot(2.0 * CLHEP::pi);
  gamma_direction.setRThetaPhi(1.0, gamma_theta/CLHEP::radian, gamma_phi / CLHEP::radian);
  G4ThreeVector gamma_momentum = gamma_E*gamma_direction;

  //evt->ProduceNewParticle( G4ThreeVector(0.0,0.0,0.0), gamma_momentum, "gamma");
  evt->ProduceNewParticle( G4ThreeVector(0.0,0.0,0.0), gamma_momentum, "gamma");


  double electronE = (fElectronEnergy + fLaserEnergy) - gamma_E; // E cons
  double momentum_e = std::sqrt(std::pow(electronE, 2) - std::pow(CLHEP::electron_mass_c2, 2)); // Actually this is to satisfy the ProduceNewParticle function.
  double theta_e = std::asin(gamma_E * std::sin(gamma_theta) / momentum_e);
  double phi_e = -gamma_phi;
  direction_e.setRThetaPhi(1.0, theta_e/CLHEP::radian, phi_e / CLHEP::radian);
  G4ThreeVector momentum_vec_e = momentum_e*direction_e;

  evt->ProduceNewParticle( G4ThreeVector(0.0,0.0,0.0), momentum_vec_e, "e-");

  G4double APV = 0.001;

  evt->SetAsymmetry(APV);
  evt->SetThCoM(0.0001);
  evt->SetEffCrossSection(0.001);

  //evt->SetQ2( 2.0*e_com*e_com*(1.0-cos(thcom)) );
  // Q2 is not actually well defined
  //evt->SetQ2( 0.0 );

}
