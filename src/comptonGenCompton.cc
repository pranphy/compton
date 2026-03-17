#include "comptonGenCompton.hh"

#include "comptonEvent.hh"
#include "comptonVertex.hh"

#include "Randomize.hh"
#include "G4SystemOfUnits.hh"

#include <math.h>


comptonGenCompton::comptonGenCompton()
    : comptonVEventGen("compton"),
    fMaxPhotonEnergy(0.0),
    fElectronEnergy(0.0),
    fLaserWavelength(0.0),
    fCrossingAngle(0.0),
    fBeamCurr(0.0),
    fLaserPower(0.0),
    fInitialAngle(0.0),
    initialized(false)
{
    fThisGenMessenger.DeclarePropertyWithUnit("beamene","GeV",fElectronEnergy,"Beam electron energy");
    fThisGenMessenger.DeclarePropertyWithUnit("beamcurr","microampere",fBeamCurr,"The beam current");
    fThisGenMessenger.DeclarePropertyWithUnit("laserwavelength","nm",fLaserWavelength,"The wavelength of laser photon");
    fThisGenMessenger.DeclarePropertyWithUnit("laserpower","watt",fLaserPower,"The power of laser");
    fThisGenMessenger.DeclarePropertyWithUnit("crossingangle","deg",fCrossingAngle,"The crossing angle");
    fThisGenMessenger.DeclarePropertyWithUnit("sigmae","micrometer",fSigmae,"The beam spot size");
    fThisGenMessenger.DeclarePropertyWithUnit("sigmap","micrometer",fSigmap,"The photon spot size");
}

comptonGenCompton::~comptonGenCompton() { }

void comptonGenCompton::Initialize(){
    auto me = CLHEP::electron_mass_c2;
    G4double gma =  fElectronEnergy/CLHEP::electron_mass_c2;

    fLaserEnergy = CLHEP::h_Planck * CLHEP::c_light / fLaserWavelength;
    fAParameter = 1 / (1 + (4*fLaserEnergy*fElectronEnergy)/ (me*me));
    fMaxPhotonEnergy = 4*fAParameter*fLaserEnergy*(gma*gma);

    for(int i = 0; i < 10000; i++) {
        G4double rho = G4double(i/10000.0);
        fCXdSig_dRho[i] = GetCrossSection(rho);
    }

    fLuminosity = GetLuminosity();
    fInitialAngle = 1.2*degree;
    initialized = true;
}

double comptonGenCompton::GetCrossSection(double rho){
    auto r0 = CLHEP::classic_electr_radius;

    G4double am1 = fAParameter - 1.0;
    G4double ap1 = fAParameter + 1.0;
    G4double term1 = rho*rho*am1*am1/(1.0 + rho*am1);
    G4double term3 = (1.0 - rho*ap1)/(1.0 + rho*am1);
    return 2 * CLHEP::pi * r0*r0 * fAParameter*(term1 +1.0 + term3*term3);
}

const double comptonGenCompton::GetLuminosity(){


    constexpr double c = 299'792'458*m/s; // m/s
    constexpr double e = e_SI*coulomb; // electron charge columb


    double lum = ( 1 + std::cos(fCrossingAngle) ) / std::sin(fCrossingAngle) *
        (fBeamCurr/e) * (fLaserPower/fLaserEnergy) * (1/c) *
        1 / ( std::sqrt(2*CLHEP::pi) * std::hypot(fSigmae,fSigmap) ) ;

    return lum;
}

double comptonGenCompton::GetRate(double rho){
    return fLuminosity * GetCrossSection(rho);
}



G4double comptonGenCompton::GetRandomRho()
{
    //G4RandGeneral GenDist(fCXdSig_dRho,10000);
    //return GenDist.shoot();
    return G4UniformRand();
}


void comptonGenCompton::SamplePhysics(comptonVertex * /*vert*/, comptonEvent *evt)
{
    if(!initialized) Initialize();

    double rho = GetRandomRho();
    double gamma_E = rho * fMaxPhotonEnergy;
    G4ThreeVector gamma_direction;
    G4ThreeVector direction_e;
    G4double gma =  fElectronEnergy/CLHEP::electron_mass_c2;
    double gamma_theta = std::sqrt( 4 * fLaserEnergy/gamma_E - 1/( fAParameter*gma*gma));
    double gamma_phi = CLHEP::RandFlat::shoot(2.0 * CLHEP::pi);
    gamma_direction.setRThetaPhi(1.0, gamma_theta/CLHEP::radian, gamma_phi / CLHEP::radian);
    G4ThreeVector gamma_momentum = gamma_E*gamma_direction;

    evt->ProduceNewParticle( G4ThreeVector(0.0,0.0,0.0), gamma_momentum, "gamma");


    double electronE = (fElectronEnergy + fLaserEnergy) - gamma_E; // E cons
    double momentum_e = std::sqrt(std::pow(electronE, 2) - std::pow(CLHEP::electron_mass_c2, 2)); // Actually this is to satisfy the ProduceNewParticle function.
    double theta_e = std::asin(gamma_E * std::sin(gamma_theta) / momentum_e);
    double phi_e = -gamma_phi;
    direction_e.setRThetaPhi(1.0, theta_e/CLHEP::radian, phi_e / CLHEP::radian);
    G4ThreeVector momentum_vec_e = momentum_e*direction_e;

    evt->ProduceNewParticle( G4ThreeVector(0.0,0.0,0.0), momentum_vec_e, "e-");



    // So basically I have to calculate luminosity first, which is
    // function of crossing angle alpha_c and the photon and beam widths.
    // and also the beam energy and current
    // Also from cross section I have dsigma/drho which is stored in fCXdSig_dRho
    // L = L(alpha_c, I, E, lambda)
    // XS = dsigma/drho at rho
    // double L =
    // rate =  L * XS

    double rate = GetRate(rho);
    double XS = GetCrossSection(rho);


    evt->SetAsymmetry(0.000);
    evt->SetThCoM(0.0000);
    evt->SetEffCrossSection(XS);
    evt->SetRate(rate); // see the rate here is in Geant4 units, writing takes care of proper unit

    //evt->SetQ2( 2.0*e_com*e_com*(1.0-cos(thcom)) );
    // Q2 is not actually well defined
    //evt->SetQ2( 0.0 );

}
