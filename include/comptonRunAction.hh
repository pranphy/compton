#ifndef comptonRunAction_h
#define comptonRunAction_h 1

#include "globals.hh"
#include "G4GenericMessenger.hh"
#include "G4UserRunAction.hh"
#include "G4Timer.hh"

class G4Run;

class comptonRunAction : public G4UserRunAction
{
  public:
    comptonRunAction();
    virtual ~comptonRunAction() = default;

  public:
    G4Run* GenerateRun();

    void BeginOfRunAction(const G4Run* aRun);
    void EndOfRunAction(const G4Run* aRun);

    void UpdateSeed(const G4long seed);

  private:
    G4GenericMessenger fMessenger{
        this,
        "/compton/",
        "Remoll properties"};

    G4Timer fTimer;

  private:
    G4int fInterval{10};
  public:
    void SetUpdateInterval(G4int interval) { fInterval = interval; };
};

#endif
