#ifndef __COMPTONEVENT_HH
#define __COMPTONEVENT_HH

/*!
   Event information class.  This needs to
   contain all the information we need to
   generate particles and provide sufficient
   output.
*/

#include <vector>

#include "G4ThreeVector.hh"

#include "comptontypes.hh"

class G4Event;
class G4PrimaryParticle;
class G4ParticleDefinition;

class comptonBeamTarget;

class comptonEvent {
    public:
	comptonEvent();
	comptonEvent(G4Event*);
	virtual ~comptonEvent();

	void ProduceNewParticle( G4ThreeVector, G4ThreeVector, G4String, G4ThreeVector spin = G4ThreeVector(0.0,0.0,0.0) );
	void ProduceNewParticle( G4ThreeVector, G4PrimaryParticle* );
	void SetEffCrossSection( G4double xs ){ fEffXs = xs; }
  void SetRate( G4double rate ){ fRate = rate;}
	G4double GetRate(){ return fRate; }
	void SetAsymmetry( G4double A ){ fAsym = A; }

	void SetQ2( G4double q2 ){ fQ2 = q2; }
	void SetW2( G4double w2 ){ fW2 = w2; }
	void SetXbj( G4double x ){ fXbj = x; }
	void SetThCoM( G4double th ){ fThCoM = th; }

	void Reset();
	void UndoLastParticle();

	G4bool EventIsSane();
	void   Print();

    private:
	comptonBeamTarget* fBeamTarget;
    public:
	const comptonBeamTarget* GetBeamTarget() const { return fBeamTarget; }
	void SetBeamTarget(comptonBeamTarget* bt) { fBeamTarget = bt; }

    public:
	// Interaction information
	G4ThreeVector fBeamMomentum;
	G4ThreeVector fBeamPolarization;
	G4ThreeVector fVertexPos;

	// Particles to be produced
	std::vector<G4ThreeVector>    fPartSpin;
	std::vector<G4ThreeVector>    fPartPos;
	std::vector<G4ThreeVector>    fPartMom;  // Generated direction (no ms)
	std::vector<G4ThreeVector>    fPartRealMom; // Direction to go to Geant4
	std::vector<G4ParticleDefinition*> fPartType;

	G4double fBeamE;
	G4double fRate;
	G4double fEffXs;
	G4double fAsym, fmAsym;

	G4double fQ2;
	G4double fW2;
	G4double fXbj;
	G4double fThCoM;

    public:
        comptonEvent_t GetEventIO() const;
        std::vector<comptonEventParticle_t> GetEventParticleIO() const;
};

#endif//__COMPTONEVENT_HH
