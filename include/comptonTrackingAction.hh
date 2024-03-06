#ifndef comptonTrackingAction_h
#define comptonTrackingAction_h 1

// geant4 includes
#include "G4Types.hh"
#include "G4GenericMessenger.hh"
#include "G4UserTrackingAction.hh"

class comptonTrackingAction : public G4UserTrackingAction
{
  public:
    comptonTrackingAction();
    virtual ~comptonTrackingAction() = default;

    void  PreUserTrackingAction(const G4Track* aTrack);
    void PostUserTrackingAction(const G4Track* aTrack);

  private:
    G4GenericMessenger fMessenger{
        this,
        "/compton/tracking/",
        "Remoll tracking properties"};

    G4int fTrackingFlag{3};
};

#endif
