#ifndef __REMOLLGENHYPERON_HH
#define __REMOLLGENHYPERON_HH 1

// compton includes
#include "comptonVEventGen.hh"
#include "comptonFileReader.hh"

class comptonGenHyperon : public comptonVEventGen {

  public:
    comptonGenHyperon();
    virtual ~comptonGenHyperon();

  private:
    void SamplePhysics(comptonVertex*, comptonEvent*);

    static comptonFileReader* fFileReader;
    comptonFileReader* GetFileReader() const;

    G4int fDebugLevel;
    std::string fFile;
    G4String fParticle;
    G4int fSkip;
    G4double fRUnit;
    G4double fPUnit;
    G4double fWUnit;
};

#endif //__REMOLLGENHYPERON_HH
