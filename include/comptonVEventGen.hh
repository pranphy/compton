#ifndef __COMPTONVEVENTGEN_HH
#define __COMPTONVEVENTGEN_HH

#include "comptontypes.hh"
#include "comptonglobs.hh"
#include "comptonVertex.hh"

#include "G4String.hh"
#include "G4ThreeVector.hh"
#include "G4GenericMessenger.hh"

/*!
   Generic base class for event generators
   This provides an interface for everyone to
   derive from.

   Ultimately this returns a comptonEvent which is
   what the PrimaryGeneratorAction is going to use and
   contains information that will go in the output.

   It needs to be aware of comptonBeamTarget and comptonRunData,
   take a generically generated event assuming ideal beam
   and transform it into what is going to be simulated.
*/

class G4ParticleGun;

class comptonEvent;
class comptonBeamTarget;
class comptonRunData;

class comptonVEventGen {
    public:
	comptonVEventGen(G4String name);
	virtual ~comptonVEventGen();

	virtual void PrintEventGen();

	comptonEvent *GenerateEvent();

	G4String GetName() { return fName; }

	void SetBeamTarget(comptonBeamTarget* bt) {
	  fBeamTarg = bt;
	}

	void SetSamplingType(SamplingType_t type) { fSamplingType = type; }
	SamplingType_t GetSamplingType() const { return fSamplingType; }

	void SetDoMultScatt( G4bool multscatt ){ fApplyMultScatt = multscatt; }

        void SetEmin(double emin) { fE_min = emin; }
        void SetEmax(double emax) { fE_max = emax; }
        void SetPhmin(double phmin) { fPh_min = phmin; }
        void SetPhmax(double phmax) { fPh_max = phmax; }
        void SetThmin(double thmin) { fTh_min = thmin; }
        void SetThmax(double thmax) { fTh_max = thmax; }

    protected:
	// Generator name
        G4String fName;

    protected:
	// Generation limits
	static G4double fThCoM_min, fThCoM_max;
	static G4double fTh_min, fTh_max;
	static G4double fPh_min, fPh_max;
	static G4double fE_min, fE_max;

  G4String fBeamPol;
public:
  const G4String GetBeamPolarization(){return fBeamPol;}

    protected:
	// Number of particles
	G4int fNumberOfParticles;
	// Particle gun
        G4ParticleGun* fParticleGun;
    public:
	// Set the number of particles
	void SetNumberOfParticles(G4int n);
	// Get a new particle gun for this generator
        G4ParticleGun* GetParticleGun() const { return fParticleGun; }

    protected:
	comptonBeamTarget* fBeamTarg;

	void PolishEvent(comptonEvent *);

	// Pure virtual function that needs to be filled out
	virtual void SamplePhysics(comptonVertex *, comptonEvent *) = 0;

    protected:

	SamplingType_t fSamplingType;
	G4bool     fApplyMultScatt;

    private:
	// Generic messenger as protected to be used in derived classes
	G4GenericMessenger fEvGenMessenger{
            this,
            "/compton/evgen/",
            "Remoll event generator properties"};
    protected:
	G4GenericMessenger fThisGenMessenger;
};


#endif//__COMPTONVEVENTGEN_HH
