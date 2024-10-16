#ifndef __COMPTONGENERICDETECTOR_HH
#define __COMPTONGENERICDETECTOR_HH

#include "G4VSensitiveDetector.hh"
#include "G4TwoVector.hh"
#include "G4Threading.hh"
#include "G4AutoLock.hh"
#include "G4Version.hh"

// Included to avoid forward declaration of collection typedef
#include "comptonGenericDetectorHit.hh"
#include "comptonGenericDetectorSum.hh"

#include <map>
#include <list>

/*!
      Default detector class.  This will record information on:

      - Primary generated hit information
      - Secondary hit information on particles not produced within the volume
      - Calorimetric data (total energy deposited)

      This requires two hit collections, one for "hits" and one for "sum"
      Summing occurs uniquely over copyIDs
*/

class G4HCofThisEvent;
class G4Step;
class G4TouchableHistory;
class G4GenericMessenger;

class comptonGenericDetectorSum;

namespace { G4Mutex comptonGenericDetectorMutex = G4MUTEX_INITIALIZER; }

class comptonGenericDetector : public G4VSensitiveDetector {

  private:

    static G4GenericMessenger* fStaticMessenger;
    static std::list<comptonGenericDetector*> fGenericDetectors;
    static void InsertGenericDetector(comptonGenericDetector* det) {
      G4AutoLock lock(&comptonGenericDetectorMutex);
      fGenericDetectors.push_back(det);
      fGenericDetectors.sort(isBefore);
    }
    static void EraseGenericDetector(comptonGenericDetector* det) {
      G4AutoLock lock(&comptonGenericDetectorMutex);
      fGenericDetectors.remove(det);
    }
    static void Sort() {
      G4AutoLock lock(&comptonGenericDetectorMutex);
      fGenericDetectors.sort(isBefore);
    }

    void PrintAll() {
      for (std::list<comptonGenericDetector*>::const_iterator
        it  = fGenericDetectors.begin();
        it != fGenericDetectors.end();
        it++) {
          (*it)->PrintEnabled();
      }
    }

    void PrintSummary(G4int det) {
      for (std::list<comptonGenericDetector*>::iterator
        it  = fGenericDetectors.begin();
        it != fGenericDetectors.end();
        it++) {
          if ((*it)->fDetNo == det)
            (*it)->PrintSummary();
      }
    }

    void SetAllEnabled() {
      for (std::list<comptonGenericDetector*>::iterator
        it  = fGenericDetectors.begin();
        it != fGenericDetectors.end();
        it++) {
          (*it)->SetEnabled();
      }
    }
    void SetAllDisabled() {
      for (std::list<comptonGenericDetector*>::iterator
        it  = fGenericDetectors.begin();
        it != fGenericDetectors.end();
        it++) {
          (*it)->SetDisabled();
      }
    }

    void SetOneEnabled(G4int det) {
      for (std::list<comptonGenericDetector*>::iterator
        it  = fGenericDetectors.begin();
        it != fGenericDetectors.end();
        it++) {
          if ((*it)->fDetNo == det)
            (*it)->SetEnabled();
      }
    }
    void SetOneDisabled(G4int det) {
      for (std::list<comptonGenericDetector*>::iterator
        it  = fGenericDetectors.begin();
        it != fGenericDetectors.end();
        it++) {
          if ((*it)->fDetNo == det)
            (*it)->SetDisabled();
      }
    }

    void SetRangeEnabled(G4TwoVector v) {
      for (std::list<comptonGenericDetector*>::iterator
        it  = fGenericDetectors.begin();
        it != fGenericDetectors.end();
        it++) {
          if ((*it)->fDetNo >= v.x() && (*it)->fDetNo <= v.y())
            (*it)->SetEnabled();
      }
    }
    void SetRangeDisabled(G4TwoVector v) {
      for (std::list<comptonGenericDetector*>::iterator
        it  = fGenericDetectors.begin();
        it != fGenericDetectors.end();
        it++) {
          if ((*it)->fDetNo >= v.x() && (*it)->fDetNo <= v.y())
            (*it)->SetDisabled();
      }
    }

    static bool isBefore(const comptonGenericDetector* left, const comptonGenericDetector* right) {
      return (left? (right? (left->fDetNo < right->fDetNo): false): true);
    }

    void SetRangeDetectorType(G4String type, G4TwoVector v) {
      for (std::list<comptonGenericDetector*>::iterator
        it  = fGenericDetectors.begin();
        it != fGenericDetectors.end();
        it++) {
          if ((*it)->fDetNo >= v.x() && (*it)->fDetNo <= v.y())
            (*it)->SetDetectorType(type);
      }
    }
    void SetOneDetectorType(G4String type, G4int det) {
      for (std::list<comptonGenericDetector*>::iterator
        it  = fGenericDetectors.begin();
        it != fGenericDetectors.end();
        it++) {
          if ((*it)->fDetNo == det)
            (*it)->SetDetectorType(type);
      }
    }

  public:
      comptonGenericDetector( G4String name, G4int detnum );
      virtual ~comptonGenericDetector();

      virtual void Initialize(G4HCofThisEvent*);
      virtual G4bool ProcessHits(G4Step*,G4TouchableHistory*);
      virtual void EndOfEvent(G4HCofThisEvent*);

      void BuildStaticMessenger();

      virtual void SetDetectorType(G4String det_type) {
        auto icompare = [](const G4String& lhs, const G4String& rhs) {
          #if G4VERSION_NUMBER < 1100
            return lhs.compareTo(rhs, G4String::ignoreCase);
          #else
            return G4StrUtil::icompare(lhs, rhs);
          #endif
        };
        if (icompare(det_type, "charge") == 0) {
          fDetectOpticalPhotons = false;
          fDetectLowEnergyNeutrals = false;
        }
        if (icompare(det_type, "all") == 0) {
          fDetectSecondaries = true;
          fDetectLowEnergyNeutrals = true;
        }
        if (icompare(det_type, "lowenergyneutral") == 0) {
          fDetectLowEnergyNeutrals = true;
        }
        if (icompare(det_type, "opticalphoton") == 0) {
          fDetectOpticalPhotons = true;
        }
        if (icompare(det_type, "boundaryhits") == 0) {
          fDetectBoundaryHits = true;
        }

        if (icompare(det_type, "incomingonly") == 0) {
          fDetectIncomingOnly = true;
        }
        if (icompare(det_type, "secondaries") == 0) {
          fDetectSecondaries = true;
        }
      }

      virtual void PrintDetectorType() {
        if (fDetectOpticalPhotons == false && fDetectLowEnergyNeutrals == false) {
          G4cout << GetName() << " detects charged particles" << G4endl;
        }
        if (fDetectSecondaries == true && fDetectLowEnergyNeutrals == true) {
          G4cout << GetName() << " detects all particles" << G4endl;
        }
        if (fDetectLowEnergyNeutrals == true) {
          G4cout << GetName() << " detects low energy neutrals" << G4endl;
        }
        if (fDetectOpticalPhotons == true) {
          G4cout << GetName() << " detects optical photons" << G4endl;
        }
        if (fDetectBoundaryHits == true) {
          G4cout << GetName() << " detects hits only on entry boundary" << G4endl;
        }
        if (fDetectSecondaries == true) {
          G4cout << GetName() << " detects secondaries" << G4endl;
        }
      }

      void SetEnabled(G4bool flag = true) {
        fEnabled = flag;
      };
      void SetDisabled(G4bool flag = true) {
        fEnabled = !flag;
      };
      void PrintEnabled() const {
        G4cout << "Det " << GetName() << " (" << fDetNo << ") "
            << (fEnabled? "enabled" : "disabled")
            << (fDetectLowEnergyNeutrals? " lowenergyneutral":"")
            << (fDetectOpticalPhotons? " opticalphoton":"")
            << (fDetectSecondaries? " secondaries":"")
            << G4endl;
      };

      void PrintSummary() const {
        for (auto it = fRunningSumMap.begin(); it != fRunningSumMap.end(); it++) {
          G4cout << "Det no " << fDetNo << ", "
                 << "copy no " << it->first << ": " << G4endl;
          it->second->PrintSummary();
        }
      };

      void  SetDetNo(G4int detno) { fDetNo = detno; }
      G4int GetDetNo() const { return fDetNo; }

    private:
      comptonGenericDetectorHitCollection *fHitColl;
      comptonGenericDetectorSumCollection *fSumColl;

      G4int fHCID, fSCID;

      std::map<int, comptonGenericDetectorSum *> fRunningSumMap;
      std::map<int, comptonGenericDetectorSum *> fSumMap;

      G4bool fDetectSecondaries;
      G4bool fDetectOpticalPhotons;
      G4bool fDetectLowEnergyNeutrals;
      G4bool fDetectBoundaryHits;
      G4bool fDetectIncomingOnly;
      //G4bool fDetectBoundaryExit;

      G4int fDetNo;

      G4bool fEnabled;

      G4GenericMessenger* fMessenger;

};

#endif//__COMPTONGENERICDETECTOR_HH
