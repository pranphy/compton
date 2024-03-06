#include "comptonEventAction.hh"
#include "comptonGenericDetectorHit.hh"
#include "comptonGenericDetectorSum.hh"
#include "comptonPrimaryGeneratorAction.hh"

#include "G4Event.hh"
#include "G4HCofThisEvent.hh"
#include "G4VHitsCollection.hh"

#include "G4RunManager.hh"

#include "comptonIO.hh"
#include "comptonEvent.hh"
#include "comptonTrackReconstruct.hh"

#include "G4Threading.hh"
#include "G4AutoLock.hh"
namespace { G4Mutex comptonEventActionMutex = G4MUTEX_INITIALIZER; }

comptonEventAction::comptonEventAction()
  : fPrimaryGeneratorAction(0) { }

comptonEventAction::~comptonEventAction() { }

void comptonEventAction::BeginOfEventAction(const G4Event* /* event */) { }

void comptonEventAction::EndOfEventAction(const G4Event* aEvent)
{
  // We collect all interaction with comptonIO in this thread for as
  // little locking as possible. This means that all the thread local
  // information must be retrieved from here.

  // Lock mutex
  G4AutoLock lock(&comptonEventActionMutex);
  comptonIO* io = comptonIO::GetInstance();

  // Store random seed
  static G4RunManager* run_manager = G4RunManager::GetRunManager();
  if (run_manager->GetFlagRandomNumberStatusToG4Event() % 2 == 1) {
    Int_t run = run_manager->GetCurrentRun()->GetRunID();
    Int_t evt = aEvent->GetEventID();
    io->SetEventSeed(run, evt, aEvent->GetRandomNumberStatus());
  }

  // Get primary event action information
  const comptonEvent* event = fPrimaryGeneratorAction->GetEvent();
  io->SetEventData(event);

  // Create track reconstruction object
  comptonTrackReconstruct track;

  // Traverse all hit collections, sort by output type
  G4HCofThisEvent *HCE = aEvent->GetHCofThisEvent();
  auto n = HCE->GetCapacity();
  for (decltype(n) hcidx = 0; hcidx < n; hcidx++) {
    G4VHitsCollection* thiscol = HCE->GetHC(hcidx);
    if (thiscol != nullptr){ // This is NULL if nothing is stored

      // Dynamic cast to test types, process however see fit and feed to IO

      ////  Generic Detector Hits ///////////////////////////////////
      if (comptonGenericDetectorHitCollection *thiscast =
          dynamic_cast<comptonGenericDetectorHitCollection*>(thiscol)) {
        for (unsigned int hidx = 0; hidx < thiscast->GetSize(); hidx++) {

	  comptonGenericDetectorHit *currentHit =
	    (comptonGenericDetectorHit *) thiscast->GetHit(hidx);

	  ////  store GEM hits for track reconstruction
	  if(currentHit->fDetID >= 501 && currentHit->fDetID <= 504){
	    track.AddHit(currentHit);
	  }
	  // non-GEM hits
	  else io->AddGenericDetectorHit(currentHit);
        }
      }

      ////  Generic Detector Sum ////////////////////////////////////
      if (comptonGenericDetectorSumCollection *thiscast =
          dynamic_cast<comptonGenericDetectorSumCollection*>(thiscol)) {
        for (unsigned int hidx = 0; hidx < thiscast->GetSize(); hidx++) {
          io->AddGenericDetectorSum((comptonGenericDetectorSum *)
                                    thiscast->GetHit(hidx));
        }
      }

    }
  }

  ////  reconstruct tracks, and store them into rootfile
  if (track.GetTrackHitSize() > 0) {

    track.ReconstructTrack();

    std::vector<comptonGenericDetectorHit*> rechits = track.GetTrack();

    for (size_t j = 0; j < rechits.size(); j++)
      io->AddGenericDetectorHit((comptonGenericDetectorHit *) rechits[j]);
  }

  // Fill tree and reset buffers
  io->FillTree();
  io->Flush();
}
