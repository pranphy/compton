#include "comptonSteppingAction.hh"

#include "G4Step.hh"
#include "G4Track.hh"
#include "G4UserLimits.hh"

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
    double zerr = 20;
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

void comptonSteppingAction::LimitStep(const G4Step* aStep){
    G4double zPos = aStep->GetPostStepPoint()->GetPosition().z();

    // Define the range of z coordinates
    double mm = CLHEP::mm;
    double mag1_pos = -7049.8*mm;
    G4double zMin = mag1_pos - 1000*mm;  // example min z coordinate
    G4double zMax = mag1_pos + 1000*mm;  // example max z coordinate

    // Check if the post step z coordinate is within the range
    if (zPos >= zMin && zPos <= zMax) {
        //std::cout<<" Between "<<zMin/mm<<"mm and "<<zMax/mm<<"mm and we got zpos="<<zPos/mm<<"mm "<<std::endl;
        //aStep->GetTrack()->SetStepLength(1*mm);
        G4Track* track = aStep->GetTrack();
        G4double stepLength = aStep->GetStepLength();
        if (stepLength > 1.0 * mm) {
            G4UserLimits* userLimits = track->GetVolume()->GetLogicalVolume()->GetUserLimits();
            if (userLimits) {
                userLimits->SetMaxAllowedStep(1 * mm); // Set your desired step length
            } else {
                auto limit = new G4UserLimits(1*mm);
                track->GetVolume()->GetLogicalVolume()->SetUserLimits(limit);
                track->SetStepLength(1.0*mm);
            }
        }
        //G4double stepLength = aStep->GetStepLength();
        //if (stepLength > 1.0 * mm) {
        //    // If the step length is greater than 1mm, set the step limit
        //    G4double newStepLength = 1.0 * mm;
        //    aStep->GetTrack()->SetStepLength(newStepLength);
        //}
    }
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
    LimitStep(aStep);
}

void comptonSteppingAction::DefineStepConnamds()
{
    fStepMessenger = new G4GenericMessenger(this,"/compton/step/","Compton Stepping Action Control Options");
    fStepMessenger->DeclareMethodWithUnit("killminenergy", "MeV", &comptonSteppingAction::SetKillMinEnergy,"Kill all particles less than passed energy");
    fStepMessenger->DeclareMethod("killpidafterz", &comptonSteppingAction::SetKillPidZPos,"Kill pid after certain z location");
}
