#ifndef __REMOLLGENFLAT_HH 
#define __REMOLLGENFLAT_HH 
/*!
 * Flat event generator
 *
 * Seamus Riordan
 * February 5, 2014
 *
*/

#include "comptonVEventGen.hh"

class comptonGenFlat : public comptonVEventGen {
    public:
	comptonGenFlat();
	virtual ~comptonGenFlat();

	void SetParticleName(const G4String& name) {
	  fParticleName = name;
	}

    private:
	void SamplePhysics(comptonVertex *, comptonEvent *);

        G4String fParticleName;
};

#endif//__REMOLLGENMOLLER_HH 
