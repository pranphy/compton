
#ifndef __REMOLLSTEPPINGACTION_HH
#define __REMOLLSTEPPINGACTION_HH

#include "G4UserSteppingAction.hh"
#include "globals.hh"

#include <set>

class G4Material;
class G4GenericMessenger;

class comptonSteppingAction : public G4UserSteppingAction
{
  public:
    comptonSteppingAction();
    virtual ~comptonSteppingAction();

    virtual void UserSteppingAction(const G4Step*);
};

#endif//__REMOLLSTEPPINGACTION_HH
