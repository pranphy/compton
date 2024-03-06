#ifndef comptonPrimaryGeneratorAction_h
#define comptonPrimaryGeneratorAction_h 1

#include "comptonBeamTarget.hh"

#include "G4VUserPrimaryGeneratorAction.hh"
#include "G4VPrimaryGenerator.hh"
#include "G4GenericMessenger.hh"
#include "G4String.hh"

#include <map>
#include <memory>

class G4ParticleGun;
class G4Event;
class comptonVEventGen;
class comptonEvent;

class comptonPrimaryGeneratorAction : public G4VUserPrimaryGeneratorAction
{
  public:
    comptonPrimaryGeneratorAction();
    virtual ~comptonPrimaryGeneratorAction();

  public:
    void GeneratePrimaries(G4Event* anEvent);

    const comptonEvent* GetEvent() const { return fEvent; }

    void SetGenerator(G4String&);

  private:
    std::map<G4String,std::shared_ptr<comptonVEventGen>> fEvGenMap;
    std::shared_ptr<comptonVEventGen> fEventGen;
    G4String fEventGenName;

    std::map<G4String,std::shared_ptr<G4VPrimaryGenerator>> fPriGenMap;
    std::shared_ptr<G4VPrimaryGenerator> fPriGen;
    G4String fPriGenName;

    G4ParticleGun* fParticleGun;

    comptonBeamTarget fBeamTarg;


    comptonEvent *fEvent;

    G4int fRateCopy;
    G4GenericMessenger fEvGenMessenger{this,"/compton/evgen/","Remoll event generator properties"};


    G4double fEffCrossSection;
};

#endif
