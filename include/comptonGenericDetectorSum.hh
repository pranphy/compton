#ifndef __COMPTONGENERICDETECTORSUM_HH
#define __COMPTONGENERICDETECTORSUM_HH

#include "G4VHit.hh"
#include "G4THitsCollection.hh"
#include "G4Allocator.hh"
#include "G4ThreeVector.hh"

#include "comptontypes.hh"

#include <vector>

#include "comptontypes.hh"

class comptonGenericDetectorSum : public G4VHit {
    public:
	comptonGenericDetectorSum(G4int, G4int);
	virtual ~comptonGenericDetectorSum();

	comptonGenericDetectorSum(const comptonGenericDetectorSum &right);
	const comptonGenericDetectorSum& operator=(const comptonGenericDetectorSum &right);
	G4int operator==(const comptonGenericDetectorSum &right) const;

	inline void *operator new(size_t);
	inline void operator delete(void *aHit);

    public:
	G4int    fDetID;
	G4int    fCopyID;
	G4double fEdep;
        G4int    fNhit;

	void AddEDep(int pid, G4ThreeVector x, double edep);
        void PrintSummary() const;

	std::map<int,comptonGenericDetectorSumByPID_t> fSumByPID;

    public:
      const comptonGenericDetectorSum_t GetGenericDetectorSumIO() const {
        comptonGenericDetectorSum_t sum;
        sum.det = fDetID;
        sum.vid = fCopyID;
        sum.edep = fEdep;
        sum.n = fNhit;
        for (std::map<int,comptonGenericDetectorSumByPID_t>::const_iterator
            it  = fSumByPID.begin();
            it != fSumByPID.end(); ++it) {
          sum.by_pid.push_back(it->second);
          sum.by_pid.back().pid = it->first;
        }
        return sum;
      }
};


typedef G4THitsCollection<comptonGenericDetectorSum> comptonGenericDetectorSumCollection;

extern G4ThreadLocal G4Allocator<comptonGenericDetectorSum>* comptonGenericDetectorSumAllocator;

inline void* comptonGenericDetectorSum::operator new(size_t){
  if (!comptonGenericDetectorSumAllocator)
    comptonGenericDetectorSumAllocator = new G4Allocator<comptonGenericDetectorSum>;
  return (void *) comptonGenericDetectorSumAllocator->MallocSingle();
}

inline void comptonGenericDetectorSum::operator delete(void *aHit){
  comptonGenericDetectorSumAllocator->FreeSingle( (comptonGenericDetectorSum*) aHit);
}

#endif//__COMPTONGENERICDETECTORSUM_HH
