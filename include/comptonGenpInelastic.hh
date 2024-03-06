#ifndef __REMOLLGENPINELASTIC_HH 
#define __REMOLLGENPINELASTIC_HH 
/*!
 * Inelastic ep event generator
 *
 * Seamus Riordan
 * February 14, 2013
 *
 * Uses Christy/Bosted parameterization
*/

#include "comptonVEventGen.hh"

class comptonGenpInelastic : public comptonVEventGen {
    public:
	comptonGenpInelastic();
	virtual ~comptonGenpInelastic();

    private:
	void SamplePhysics(comptonVertex *, comptonEvent *);

};

#endif//__REMOLLGENMOLLER_HH 
