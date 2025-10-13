#include "comptonPrimaryGeneratorAction.hh"

#include "G4Event.hh"
#include "G4ParticleGun.hh"
#include "G4ParticleDefinition.hh"
#include "G4Version.hh"

#include "comptonVEventGen.hh"
#include "comptonEvent.hh"
#include "comptonRun.hh"
#include "comptonRunData.hh"
#include "comptontypes.hh"

#include "comptonGenCompton.hh"
#include "comptonGenBeam.hh"
#include "comptonGenLaser.hh"
#include "comptonGenExternal.hh"

#include <memory>

comptonPrimaryGeneratorAction::comptonPrimaryGeneratorAction()
    : fEventGen(0),fParticleGun(0),fEvent(0),fRateCopy(0),fEffCrossSection(0)
{
    static bool has_been_warned = false;
    if (! has_been_warned) {
        G4cout << "compton: All possible event generators are instantiated every time." << G4endl;
        G4cout << "compton: This means some will not find necessary input files or" << G4endl;
        G4cout << "compton: print other information in the next few lines." << G4endl;
        has_been_warned = true;
    }

    // Populate map with all possible event generators
    fEvGenMap["compton"] = std::make_shared<comptonGenCompton>();
    fEvGenMap["beam"] = std::make_shared<comptonGenBeam>();
    fEvGenMap["laser"] = std::make_shared<comptonGenLaser>();
    fEvGenMap["external"] = std::make_shared<comptonGenExternal>();

    // Default generator
    G4String default_generator = "beam";
    SetGenerator(default_generator);

    // Create event generator messenger
    fEvGenMessenger.DeclareMethod("set",&comptonPrimaryGeneratorAction::SetGenerator,"Select physics generator");
    fEvGenMessenger.DeclarePropertyWithUnit("sigma","picobarn",fEffCrossSection,"Set effective cross section");
    fEvGenMessenger.DeclareProperty("copyRate",fRateCopy,"ExtGen: copy rate from previous sim");
}

comptonPrimaryGeneratorAction::~comptonPrimaryGeneratorAction()
{
}

void comptonPrimaryGeneratorAction::SetGenerator(G4String& genname)
{
    // Set generator to null
    fEventGen = 0;

    // Find event generator
    auto evgen = fEvGenMap.find(genname);
    if (evgen != fEvGenMap.end()) {
        //G4cout << "Setting generator to " << genname << G4endl;
        fEventGen = evgen->second;
        fEventGenName = evgen->first;
        fParticleGun = fEventGen->GetParticleGun();
    }

}

void comptonPrimaryGeneratorAction::GeneratePrimaries(G4Event* anEvent)
{
    if (!fEventGen) {
        G4cerr << __FILE__ << " line " << __LINE__ << " - No event generator found." << G4endl;
        exit(1);
    }

    // Delete old primary event
    if (fEvent != nullptr) {
        delete fEvent;
        fEvent = 0;
    }


    // 2. Using event generator interface
    if (fEventGen) {

        // Helper function
        auto contains = [](const G4String& lhs, const G4String& rhs) {
            #if G4VERSION_NUMBER < 1100
                    return lhs.contains(rhs);
            #else
                    return G4StrUtil::contains(lhs, rhs);
            #endif
        };

        // Set beam polarization
        const G4String fBeamPol = fEventGen->GetBeamPolarization();
        G4ThreeVector cross(0,0,2);
        if (fBeamPol == "0") cross = G4ThreeVector(0,0,0);
        else {
            if     (contains(fBeamPol, "V")) cross = G4ThreeVector(1,0,0);
            else if(contains(fBeamPol, "H")) cross = G4ThreeVector(0,1,0);
            if (contains(fBeamPol, "-")) cross *= -1;
        }

        // Create new primary event
        fEvent = fEventGen->GenerateEvent();
        for (unsigned int pidx = 0; pidx < fEvent->fPartType.size(); pidx++) {

            double p = fEvent->fPartRealMom[pidx].mag();
            double m = fEvent->fPartType[pidx]->GetPDGMass();
            double kinE = sqrt(p*p + m*m) - m;

            fParticleGun->SetParticleDefinition(fEvent->fPartType[pidx]);
            fParticleGun->SetParticleEnergy(kinE);
            fParticleGun->SetParticlePosition(fEvent->fPartPos[pidx]);
            fParticleGun->SetParticleMomentumDirection(fEvent->fPartRealMom[pidx].unit());

            G4ThreeVector pol(0,0,0);
            fParticleGun->SetParticlePolarization(pol);

            fParticleGun->GeneratePrimaryVertex(anEvent);
        }
    }

}
