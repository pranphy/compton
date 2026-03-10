#ifndef __COMPTONGENCOMPTON_HH
#define __COMPTONGENCOMPTON_HH
/*!
 * Compton event generator for.
 *
 * Prakash Gautam
 * 2024-06-14
 *
*/

#include "comptonVEventGen.hh"

class comptonGenCompton : public comptonVEventGen {
  public:
    comptonGenCompton();
    virtual ~comptonGenCompton();


  private:

    G4double fCXdSig_dRho[10000];
    G4double fMaxPhotonEnergy;
    G4double fBeamCurr;
    G4double fLaserPower;
    G4double fCrossingAngle;
    G4double fSigmae;
    G4double fSigmap;
    G4double fElectronEnergy;
    G4double fLaserEnergy;
    G4double fLaserWavelength;
    G4double fAParameter;
    G4double fLuminosity;
    double get_max_k();

    G4double GetRandomRho();
    void SamplePhysics(comptonVertex *, comptonEvent *);
    double cross_section(double rho);
    double get_rate(double rho);
    const double get_luminosity();
    void Initialize();
    bool initialized;
};

#endif//__COMPTONGENCOMPTON_HH
