#include "comptonGenCompton.hh"

#include "comptonEvent.hh"
#include "comptonVertex.hh"

#include "Randomize.hh"
#include "G4SystemOfUnits.hh"

#include "comptontypes.hh"

#include <math.h>


comptonGenCompton::comptonGenCompton()
: comptonVEventGen("compton"),
   fMaxPhotonEnergy(0.0),
   fElectronEnergy(11*GeV),
   fLaserWavelength(532*nm)
{
    auto me = CLHEP::electron_mass_c2;
    G4double gma =  fElectronEnergy/CLHEP::electron_mass_c2;

    fLaserEnergy = CLHEP::h_Planck * CLHEP::c_light / fLaserWavelength;
    fAParameter = 1 / (1 + (4*fLaserEnergy*fElectronEnergy)/ (me*me));
    //fMaxPhotonEnergy = fElectronEnergy * (1 - fAParameter);
    fMaxPhotonEnergy = 4*fAParameter*fLaserEnergy*(gma*gma);
    //printf("fLaserEnergy = %.2e MeV fMaxPhotonEnergy = %.2eMeV \n",fLaserEnergy,fMaxPhotonEnergy);

    for(int i = 0; i < 10000; i++) {
        G4double rho = G4double(i/10000.0);
        fCXdSig_dRho[i] = cross_section(rho);
        //of<<fCXdSig_dRho[i]<<std::endl;
    }

    fLuminosity = get_luminosity();
}

comptonGenCompton::~comptonGenCompton() { }

double comptonGenCompton::cross_section(double rho){
    auto r0 = CLHEP::classic_electr_radius;
    //printf("Laser The classic electron radius is %.3e vs %.3em\n",r0,r0/CLHEP::m);

    G4double am1 = fAParameter - 1.0;
    G4double ap1 = fAParameter + 1.0;
    G4double term1 = rho*rho*am1*am1/(1.0 + rho*am1);
    G4double term3 = (1.0 - rho*ap1)/(1.0 + rho*am1);
    return 2 * CLHEP::pi * r0*r0 * fAParameter*(term1 +1.0 + term3*term3);
}

double comptonGenCompton::get_luminosity(){
    double alpha_c = 3.4*degree;
    double Ie = 65*microampere; // current in amp should come from macros
    double Ig = 1*watt; // laser power in watts
    double sige = 60*um; // electron beam sigma 60um
    double sigg = 45*um; // gamma beam sigma 45um
    double c = 3e8*m/s; // m/s
    double e = e_SI*coulomb; // electron charge columb
    double k0 = fLaserEnergy;
    double n_e = Ie/e;
    //printf(" Laser n_e = %.2e and n_g = %.2e  Ig/k0 = %.3e \n",n_e,Ig/k0,Ig/k0);


    double lum = ( 1 + std::cos(alpha_c) ) / std::sin(alpha_c) *
        (Ie/e) * (Ig/k0) * (1/c) *
        1 / ( std::sqrt(2*CLHEP::pi) * std::hypot(sige,sigg) ) ; //* ( 1e28);

    //printf(" Laser luminosity L = %.5f (barnes-s) \n",lum*(barn*s));
    return lum;
}

double comptonGenCompton::get_rate(double rho){
    return fLuminosity * cross_section(rho);
}



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



  // So basically I have to calculate luminosity first, which is
  // function of crossing angle alpha_c and the photon and beam widths.
  // and also the beam energy and current
  // Also from cross section I have dsigma/drho which is stored in fCXdSig_dRho
  // L = L(alpha_c, I, E, lambda)
  // XS = dsigma/drho at rho
  // double L =
  // rate =  L * XS

  double rate = get_rate(rho);
  double XS = cross_section(rho);


  evt->SetAsymmetry(0.000);
  evt->SetThCoM(0.0000);
  evt->SetEffCrossSection(XS);
  evt->SetRate(rate); // see the rate here is in Geant4 units writing takes care of proper unit
  //printf("Laser: The rate is rate = %.3e  in second rate=%.3e/s\n",rate, rate*CLHEP::s);

  //evt->SetQ2( 2.0*e_com*e_com*(1.0-cos(thcom)) );
  // Q2 is not actually well defined
  //evt->SetQ2( 0.0 );

}
