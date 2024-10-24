#ifndef comptonIO_HH
#define comptonIO_HH

#include "TROOT.h"
#include "TObject.h"

#include "G4Run.hh"
#include "G4Threading.hh"
#include "G4AutoLock.hh"

#include "comptontypes.hh"
#include "comptonSystemOfUnits.hh"

#include "G4String.hh"
#include "G4GenericMessenger.hh"

#include <map>
#include <vector>
#include <fstream>

class TFile;
class TTree;

class comptonGenericDetectorHit;
class comptonGenericDetectorSum;
class comptonEvent;

namespace { G4Mutex comptonIOMutex = G4MUTEX_INITIALIZER; }

//FIXME: forward declares not possible since xerces uses a
// namespace alias and requires upstream knowledge or a pre-
// processor directive, which in turn requires another header
// so there's no gain...
//#include <xercesc/dom/DOMElement.hpp>
// or
#include <xercesc/util/XercesDefs.hpp>
XERCES_CPP_NAMESPACE_BEGIN
class DOMElement;
XERCES_CPP_NAMESPACE_END

#define __FILENAMELEN 255

// Helper class to save seed and provide Save functionality
class comptonSeed_t: public TObject {
  private:
    Int_t fRunNo; //< run number
    Int_t fEvtNo; //< evt number
    TString fSeed; //< random engine state, a.k.a. seed (but not really)
  public:
    // Default constructor
    comptonSeed_t(): TObject() { fRunNo = 0; fEvtNo = 0; fSeed = ""; };
    // Copy constructor (not implemented)
    comptonSeed_t(const comptonSeed_t& orig);
    // Virtual destructor
    virtual ~comptonSeed_t() { };
    // Setter for run, evt, seed
    void SetSeed(const Int_t& run, const Int_t& evt, const TString& seed)
    { fRunNo = run; fEvtNo = evt; fSeed = seed; };
    // Assignment operator (not implemented)
    comptonSeed_t& operator=(const comptonSeed_t& orig);
    // Save function for use in ROOT tree
    int Save() const {
      std::stringstream name;
      name << "run" << fRunNo << "evt" << fEvtNo << ".state";
      std::ofstream file(name.str());
      file << fSeed;
      return 1;
    };
  ClassDef(comptonSeed_t,1);
};

class comptonIO {
    private:
        // Singleton pointer
        static comptonIO* gInstance;
        // Private constructor
        comptonIO(const G4String& outputfile);

    public:
        // Public destructor
        virtual ~comptonIO();
        // Static instance getter
        static comptonIO* GetInstance(const G4String& outputfile = "comptonout.root");

	void SetFilename(const G4String& name) { fFilename = name; }
	G4String GetFilename() const { return fFilename; }

	void FillTree();
	void Flush();
	void WriteTree();

	void WriteRun();

	void InitializeTree();

	void GrabGDMLFiles( G4String fn );

        void RegisterDetector(G4String lvname, G4String sdname, G4int no) {
          G4AutoLock lock(&comptonIOMutex);
          static std::map<G4String,G4int> fDetLVMap;
          if (fDetLVMap.count(lvname) == 0) {
            fDetLVMap[lvname] = no;
            fDetLVNos.push_back(no);
            fDetLVNames += (fDetLVNames.size() > 0? ":": "") + lvname + "/I";
          }
          static std::map<G4String,G4int> fDetSDMap;
          if (fDetSDMap.count(sdname) == 0) {
            fDetSDMap[sdname] = no;
            fDetSDNos.push_back(no);
            fDetSDNames += (fDetSDNames.size() > 0? ":": "") + sdname + "/I";
          }
        }

    private:
	TFile *fFile;
	TTree *fTree;

        G4GenericMessenger fMessenger{this,"/compton/","Remoll properties"};

        G4String fFilename;

	std::vector<G4String> fGDMLFileNames;
	std::vector<G4String> fXMLFileNames;

	void SearchGDMLforFiles(G4String );
	void TraverseChildren(  xercesc::DOMElement * );

	//  Interfaces and buffers to the tree
	//  This is potentially going to get very long...

	// Event data
    public:
	void SetEventSeed(const Int_t& run, const Int_t& evt, const G4String& seed) {
          fSeed.SetSeed(run, evt, seed);
        }

	void SetEventData(const comptonEvent *);
    private:

	// Units
	comptonUnits_t fUnits;

        // Detectors
        std::vector<Int_t> fDetLVNos;
        G4String fDetLVNames;
        std::vector<Int_t> fDetSDNos;
        G4String fDetSDNames;

	// Event data
	Double_t fRate;
	comptonSeed_t fSeed;
	comptonEvent_t fEv;
	comptonBeamTarget_t fBm;

        // Event particles
    public:
        void SetEventData(comptonEvent *);
    private:
        std::vector<comptonEventParticle_t> fEvPart;

	//  GenericDetectorHit
    public:
	void AddGenericDetectorHit(comptonGenericDetectorHit *);
    private:
	std::vector<comptonGenericDetectorHit_t> fGenDetHit;

	//  GenericDetectorSum
    public:
	void AddGenericDetectorSum(comptonGenericDetectorSum *);
    private:
        std::vector<comptonGenericDetectorSum_t> fGenDetSum;
};

#endif//comptonIO_HH
