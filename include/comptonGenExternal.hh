/*
 * comptonGenExternal.hh
 *
 *  Created on: Mar 17, 2017
 *      Author: wdconinc
 */

#ifndef __COMPTONGENEXTERNAL_HH
#define __COMPTONGENEXTERNAL_HH

/*!
 * External event generator
 *
 * This event generator reads events from an external ROOT file, structured
 * like the compton output ROOT files, and throws hits stored in a particular
 * detector ID (to be specified through a messenger class)
 *
*/

// Base class headers
#include "comptonVEventGen.hh"

// System headers
#include <vector>

#include "G4AutoLock.hh"
#include "Randomize.hh"

// Forward declarations
class TFile;
class TTree;
struct comptonEvent_t;
struct comptonGenericDetectorHit_t;

// Class definition
class comptonGenExternal : public comptonVEventGen {

    public:
        comptonGenExternal();
        virtual ~comptonGenExternal();

        void SetGenExternalFile(G4String& filename);
        void SetGenExternalZOffset(G4double tempzOffset) {
            fzOffset = tempzOffset;
        }
        void SetGenExternalDetID(const G4int detid) {
            fDetectorID = detid;
        }
        void SetGenExternalEntry(const G4int firstEventID){
            fEntry = ((firstEventID >= 0)? firstEventID : G4RandFlat::shoot(fEntries));
        }

    private:
        void SamplePhysics(comptonVertex *, comptonEvent *);

        // External event file and tree, entry number
        static TFile* fFile;
        static TTree* fTree;
        static Int_t fEntry, fEntries;

        // Event and hit structures
        static comptonEvent_t* fEvent;
        static std::vector<comptonGenericDetectorHit_t>* fHit;

        G4double fzOffset;
        G4double rate;

        // Detector ID to consider
        G4int fDetectorID;
        G4int fLoopID;

};

#endif //__COMPTONGENEXTERNAL_HH
