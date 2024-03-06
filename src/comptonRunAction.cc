#include "comptonRunAction.hh"

#include "G4RunManager.hh"
#include "G4Timer.hh"

#include "comptonIO.hh"
#include "comptonRun.hh"
#include "comptonRunData.hh"
#include "comptonBeamTarget.hh"

#include "G4Threading.hh"
#include "G4AutoLock.hh"
namespace { G4Mutex comptonRunActionMutex = G4MUTEX_INITIALIZER; }

comptonRunAction::comptonRunAction()
{
  fMessenger.DeclareMethod(
      "seed",
      &comptonRunAction::UpdateSeed,
      "Set random engine seed")
      .SetParameterName("seed", false)
      .SetStates(G4State_PreInit,G4State_Idle);
  fMessenger.DeclareMethod(
      "interval",
      &comptonRunAction::SetUpdateInterval,
      "Print this many progress points (i.e. 100 -> every 1%)")
      .SetParameterName("interval", false)
      .SetStates(G4State_PreInit,G4State_Idle);

  // Store random status before primary particle generation
  G4RunManager::GetRunManager()->StoreRandomNumberStatusToG4Event(1);
}

void comptonRunAction::UpdateSeed(const G4long seed)
{
  G4Random::setTheSeed(seed);
  comptonRun::GetRunData()->SetSeed(seed);
  G4cout << "Random seed set to " << seed << G4endl;
}

G4Run* comptonRunAction::GenerateRun()
{
  return new comptonRun();
}

void comptonRunAction::BeginOfRunAction(const G4Run* run)
{
  // Cast into comptonRun
  const comptonRun* aRun = static_cast<const comptonRun*>(run);

  // Print progress
  G4int evts_to_process = aRun->GetNumberOfEventToBeProcessed();
  G4RunManager::GetRunManager()->SetPrintProgress((evts_to_process > fInterval)
                                                  ? evts_to_process/ fInterval
                                                  : 1);

  if (IsMaster())
  {
    G4cout << "### Run " << aRun->GetRunID() << " start." << G4endl;

    fTimer.Start();

    G4AutoLock lock(&comptonRunActionMutex);
    comptonIO* io = comptonIO::GetInstance();
    io->InitializeTree();

    comptonRunData *rundata = comptonRun::GetRunData();
    rundata->SetNthrown( aRun->GetNumberOfEventToBeProcessed() );
  }
}

void comptonRunAction::EndOfRunAction(const G4Run* run)
{
  // Cast into comptonRun
  const comptonRun* aRun = static_cast<const comptonRun*>(run);

  if (IsMaster())
  {
      fTimer.Stop();

      G4cout << "### Run " << aRun->GetRunID() << " ended "
             << "(" << fTimer.GetUserElapsed() << "s)." << G4endl;

      G4AutoLock lock(&comptonRunActionMutex);
      comptonIO* io = comptonIO::GetInstance();
      io->WriteTree();
  }
}
