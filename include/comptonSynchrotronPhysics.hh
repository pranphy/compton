#pragma once

#include "G4DecayPhysics.hh"

class comptonSynchrotronPhysics: public G4VPhysicsConstructor {
public:
    comptonSynchrotronPhysics(G4int verbose = 0, const G4String& name = "comptonSynchrotronPhysics");
    virtual ~comptonSynchrotronPhysics();
protected:
    void ConstructParticle();
    void ConstructProcess();
    void ConstructSync();
    void ConstructEM();
private:
    comptonSynchrotronPhysics(const comptonSynchrotronPhysics& right) = delete;
    comptonSynchrotronPhysics& operator=(const comptonSynchrotronPhysics& right) = delete;
    G4bool fSRType;
    G4DecayPhysics* fDecayPhysics;
};
