#ifndef __COMPTONVERTEX_HH
#define __COMPTONVERTEX_HH

#include "G4ThreeVector.hh"

/*!
  Vertex information that only the
  user defined generators will see
*/

class G4Material;

class comptonVertex {
    public:
	comptonVertex();
	virtual ~comptonVertex();

	G4double   GetBeamEnergy()      const { return fBeamEnergy; }
	G4double   GetRadiationLength() const { return fRadiationLength; }
	const G4Material* GetMaterial() const { return fMaterial; }

    public:
	G4double fBeamEnergy;
	G4double fRadiationLength;
	G4Material* fMaterial;
};

#endif//__COMPTONVERTEX_HH
