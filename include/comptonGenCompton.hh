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
    G4double fInitialAngleMin;
    G4double fInitialAngleMax;

    G4double GetRandomRho();
    void SamplePhysics(comptonVertex *, comptonEvent *);
    double GetCrossSection(double rho);
    double GetRate(double rho);
    const double GetLuminosity();
    void Initialize();
    bool initialized;
};

#endif//__COMPTONGENCOMPTON_HH
