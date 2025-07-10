
#ifndef __COMPTONSTEPPINGACTION_HH
#define __COMPTONSTEPPINGACTION_HH

#include "G4UserSteppingAction.hh"
#include "G4GenericMessenger.hh"

class G4Material;

class comptonSteppingAction : public G4UserSteppingAction
{
  public:
    comptonSteppingAction();
    virtual ~comptonSteppingAction();
    G4GenericMessenger* fStepMessenger;
    void DefineStepConnamds();
    void SetKillMinEnergy(G4double);
    void SetKillPidZPos(G4int, G4double);

    G4bool fKillPidZ, fKillMinEnergy;
    G4double fKillZPos, fKillPid, fMinEnergy, fKillZErr;

    virtual void UserSteppingAction(const G4Step*);
};

#endif//__COMPTONSTEPPINGACTION_HH
