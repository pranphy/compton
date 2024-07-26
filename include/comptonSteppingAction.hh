
#ifndef __COMPTONSTEPPINGACTION_HH
#define __COMPTONSTEPPINGACTION_HH

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

#endif//__COMPTONSTEPPINGACTION_HH
