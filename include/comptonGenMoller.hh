#ifndef __REMOLLGENMOLLER_HH 
#define __REMOLLGENMOLLER_HH 
/*!
 * Moller (ee) event generator
 *
 * Seamus Riordan
 * February 4, 2013
 *
 * Based heavily on previous work from mollersim
*/

#include "comptonVEventGen.hh"

class comptonGenMoller : public comptonVEventGen {
    public:
	comptonGenMoller();
	virtual ~comptonGenMoller();

    private:
	void SamplePhysics(comptonVertex *, comptonEvent *);
};

#endif//__REMOLLGENMOLLER_HH 
