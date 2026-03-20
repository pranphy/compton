#include <math.h>

#include <comptonEvent.hh>
#include <comptonVertex.hh>
#include <G4tgbRotationMatrix.hh>

#include <Randomize.hh>
#include <G4SystemOfUnits.hh>

#include "comptonGenCompton.hh"



comptonGenCompton::comptonGenCompton()
    : comptonVEventGen("compton"),
    fMaxPhotonEnergy(0.0),
    fElectronEnergy(0.0),
    fLaserWavelength(0.0),
    fCrossingAngle(0.0),
    fBeamCurr(0.0),
    fLaserPower(0.0),
    fInitialAngleMin(0.0),
    fInitialAngleMax(0.0),
    initialized(false)
{
    fThisGenMessenger.DeclarePropertyWithUnit("beamene","GeV",fElectronEnergy,"Beam electron energy");
    fThisGenMessenger.DeclarePropertyWithUnit("beamcurr","microampere",fBeamCurr,"The beam current");
    fThisGenMessenger.DeclarePropertyWithUnit("laserwavelength","nm",fLaserWavelength,"The wavelength of laser photon");
    fThisGenMessenger.DeclarePropertyWithUnit("laserpower","watt",fLaserPower,"The power of laser");
    fThisGenMessenger.DeclarePropertyWithUnit("crossingangle","deg",fCrossingAngle,"The crossing angle");
    fThisGenMessenger.DeclarePropertyWithUnit("ethmin","milliradian",fInitialAngleMin,"The minimum polar agnle of beam");
    fThisGenMessenger.DeclarePropertyWithUnit("ethmax","milliradian",fInitialAngleMax,"The maximum polar angle of beam");
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

    double theta0 = std::acos(CLHEP::RandFlat::shoot(std::cos(fInitialAngleMin), std::cos(fInitialAngleMax)));
    double phi0   = 0.0;

    G4RotationMatrix R_to_ez;
    R_to_ez.rotateZ(-phi0);
    R_to_ez.rotateY(-theta0);

    // Inverse rotation (back to lab)
    G4RotationMatrix R_to_lab = R_to_ez.inverse();


    double rho = GetRandomRho();
    double E_gamma = rho * fMaxPhotonEnergy;
    G4double gma = fElectronEnergy / CLHEP::electron_mass_c2;

    double gamma_theta = std::sqrt( 4.0 * fLaserEnergy/E_gamma - 1.0/(fAParameter * gma * gma) );
    double gamma_phi = CLHEP::RandFlat::shoot(2.0 * CLHEP::pi);
    G4ThreeVector gamma_dir_ez;
    gamma_dir_ez.setRThetaPhi(1.0, gamma_theta/CLHEP::radian, gamma_phi/CLHEP::radian);
    G4ThreeVector momentum_gamma_ez = E_gamma * gamma_dir_ez;
    G4ThreeVector momentum_gamma_lab = R_to_lab * momentum_gamma_ez;
    evt->ProduceNewParticle(G4ThreeVector(0,0,0), momentum_gamma_lab, "gamma");


    double E_e = (fElectronEnergy + fLaserEnergy) - E_gamma;
    double momentum_e = std::sqrt(E_e*E_e - CLHEP::electron_mass_c2*CLHEP::electron_mass_c2);
    double theta_e = std::asin(E_gamma / momentum_e * std::sin(gamma_theta) );
    double phi_e = -gamma_phi;
    G4ThreeVector e_dir_ez;
    e_dir_ez.setRThetaPhi(1.0, theta_e/CLHEP::radian, phi_e/CLHEP::radian);
    G4ThreeVector momentum_e_ez = momentum_e * e_dir_ez;
    G4ThreeVector momentum_e_lab = R_to_lab * momentum_e_ez;
    evt->ProduceNewParticle(G4ThreeVector(0,0,0), momentum_e_lab, "e-");


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

}

