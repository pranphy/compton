#ifndef __COMPTONGENBEAM_HH
#define __COMPTONGENBEAM_HH

#include "comptonVEventGen.hh"

class comptonGenBeam : public comptonVEventGen {
  public:
    comptonGenBeam();
    virtual ~comptonGenBeam();


    enum EOriginModel {
      kOriginModelFlat,
      kOriginModelGauss
    };

    EOriginModel GetOriginModelFromString(G4String model) const;


    G4double GetSpread(G4double spread, EOriginModel model);
    G4ThreeVector GetSpread(G4ThreeVector spread,
      EOriginModel x = kOriginModelFlat,
      EOriginModel y = kOriginModelFlat,
      EOriginModel z = kOriginModelFlat);

    void SetDirectionIsotropic();


    void SetPolarizationX(double sx);
    void SetPolarizationY(double sy);
    void SetPolarizationZ(double sz);

    void SetPartName(G4String& name);

  private:
    void SamplePhysics(comptonVertex *, comptonEvent *);

    G4ThreeVector fOriginMean;
    G4ThreeVector fOriginSpread;
    EOriginModel  fOriginModelX, fOriginModelY, fOriginModelZ;
    G4ThreeVector fDirection;
    G4bool        fIsotropic;
    G4ThreeVector fCorrelation;
    G4ThreeVector fPolarization;

    Double_t fBeamCurrent;
    Double_t fBeamEnergy;

    G4String fParticleName;
};

#endif//__COMPTONGENBEAM_HH
