#ifndef __COMPTONGENLASER_HH
#define __COMPTONGENLASER_HH
/*!
 * Laser event generator for.
 *
 * Prakash Gautam
 * 2024-06-14
 *
*/

#include "comptonVEventGen.hh"

class comptonGenLaser : public comptonVEventGen {
  public:
    comptonGenLaser();
    virtual ~comptonGenLaser();


  private:

    G4double fMaxPhotonEnergy;
    G4double fLaserEnergy;
    G4double fLaserWavelength;
    G4ThreeVector fOriginMean;

    void SamplePhysics(comptonVertex *, comptonEvent *);
};

#endif//__COMPTONGENLASER_HH
