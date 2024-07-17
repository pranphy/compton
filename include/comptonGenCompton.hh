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
    G4double fElectronEnergy;
    G4double fLaserEnergy;
    G4double fLaserWavelength;
    G4double fAParameter;
    double get_max_k();

    G4double GetRandomRho();
    void SamplePhysics(comptonVertex *, comptonEvent *);
};

#endif//__COMPTONGENCOMPTON_HH
