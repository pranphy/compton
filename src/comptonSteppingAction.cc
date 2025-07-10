#include "comptonSteppingAction.hh"

#include "G4Step.hh"
#include "G4Track.hh"

comptonSteppingAction::comptonSteppingAction():
    fStepMessenger(nullptr),
    fKillPidZ(false),
    fKillMinEnergy(false),
    fKillZPos(-999999),
    fMinEnergy(1*CLHEP::MeV)
{
    DefineStepConnamds();
}

comptonSteppingAction::~comptonSteppingAction()
{
    if(fStepMessenger) delete fStepMessenger;
}

void comptonSteppingAction::SetKillPidZPos(G4int pid, G4double zpos){
    double zerr = 200;
    fKillZPos = zpos;
    fKillZErr = zerr;
    fKillPid = pid;
    fKillPidZ= true;
}

void comptonSteppingAction::SetKillMinEnergy(G4double energy)
{
    fMinEnergy = energy;
    fKillMinEnergy = true;
}


void comptonSteppingAction::UserSteppingAction(const G4Step *aStep)
{
    G4Track* track = aStep->GetTrack();
    if(fKillMinEnergy)
    {
        if(  track->GetTotalEnergy() < fKillMinEnergy) track->SetTrackStatus(fStopAndKill);
    }

    //G4ParticleDefinition* particle = track->GetDefinition()->GetPDGEncoding();
    //double pid = particle->GetPDGEncoding();
    double pid = track->GetDefinition()->GetPDGEncoding();

    double zpos = track->GetStep()->GetPreStepPoint()->GetPosition().z();
    if(fKillPidZ and pid == fKillPid)
    {
        double minz = fKillZPos - fKillZErr, maxz = fKillZPos + fKillZErr;
        if ( zpos > minz and zpos <  maxz)
            track->SetTrackStatus(fStopAndKill);
    }

}

void comptonSteppingAction::DefineStepConnamds()
{
    fStepMessenger = new G4GenericMessenger(this,"/compton/step/","Compton Stepping Action Control Options");
    fStepMessenger->DeclareMethodWithUnit("killminenergy", "MeV", &comptonSteppingAction::SetKillMinEnergy,"Kill all particles less than passed energy");
    fStepMessenger->DeclareMethod("killpidafterz", &comptonSteppingAction::SetKillPidZPos,"Kill pid after certain z location");
}
