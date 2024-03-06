#ifndef __REMOLLGENPELASTIC_HH 
#define __REMOLLGENPELASTIC_HH 
/*!
 * ep elastic event generator
 *
 * Seamus Riordan
 * February 12, 2013
 *
 * Based heavily on previous work from mollersim
*/

#include "comptonVEventGen.hh"

class comptonGenpElastic : public comptonVEventGen {
    public:
	comptonGenpElastic();
	virtual ~comptonGenpElastic();

    private:
	void SamplePhysics(comptonVertex *, comptonEvent *);

	G4double RadProfile(G4double,G4double);
	G4double EnergNumInt(G4double,G4double,G4double);
};

#endif//__REMOLLGENPELASTIC_HH 
