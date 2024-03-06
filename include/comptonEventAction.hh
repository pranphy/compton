#ifndef comptonEventAction_h
#define comptonEventAction_h 1

#include "G4UserEventAction.hh"
#include "G4Timer.hh"

#include "globals.hh"

class G4Event;

class comptonPrimaryGeneratorAction;
class comptonTrackReconstruct;

class comptonEventAction : public G4UserEventAction
{
  public:
    comptonEventAction();
    virtual ~comptonEventAction();

  public:
    virtual void BeginOfEventAction(const G4Event*);
    virtual void EndOfEventAction(const G4Event*);

  private:
    // Pointer to primary generator action
    comptonPrimaryGeneratorAction* fPrimaryGeneratorAction;
    comptonTrackReconstruct* rTrack;

    // Timer for benchmarking of simulation time per event
    G4Timer fTimer;

  public:
    // Setter for primary generator action
    void SetPrimaryGeneratorAction(comptonPrimaryGeneratorAction* action) {
      fPrimaryGeneratorAction = action;
    }

};

#endif
