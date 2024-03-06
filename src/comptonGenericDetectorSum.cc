#include "comptonGenericDetectorSum.hh"

#include "comptonSystemOfUnits.hh"

G4ThreadLocal G4Allocator<comptonGenericDetectorSum>* comptonGenericDetectorSumAllocator = 0;

comptonGenericDetectorSum::comptonGenericDetectorSum(int detid, int copyid)
: fDetID(detid),fCopyID(copyid),fEdep(0.0),fNhit(0) { }

comptonGenericDetectorSum::~comptonGenericDetectorSum() { }

void comptonGenericDetectorSum::PrintSummary() const
{
  G4cout << "all: edep " << fEdep/MeV << " MeV in " << fNhit << " hits:" << G4endl;
  for (auto it = fSumByPID.begin(); it != fSumByPID.end(); it++) {
    G4cout << "pid " << it->first << ": "
           << "edep " << it->second.edep/MeV << " MeV"
           << " in " << it->second.n << " hits"
           << " (avg " << it->second.edep/MeV / it->second.n << " MeV / hit)"
           << G4endl;
  }
}

void comptonGenericDetectorSum::AddEDep(int pid, G4ThreeVector pos, double edep)
{
  fNhit += 1;
  fEdep += edep;

  if (fSumByPID.count(pid) == 0)
    fSumByPID[pid] = { 0 };

  G4double oldedep = fSumByPID[pid].edep;
  fSumByPID[pid].edep += edep;
  fSumByPID[pid].n++;
  G4double newedep = fSumByPID[pid].edep;

  if (newedep > 0.0) { // avoid division by zero for first hit with zero edep
    fSumByPID[pid].x = (oldedep * fSumByPID[pid].x + edep * pos.x()) / newedep;
    fSumByPID[pid].y = (oldedep * fSumByPID[pid].y + edep * pos.y()) / newedep;
    fSumByPID[pid].z = (oldedep * fSumByPID[pid].z + edep * pos.z()) / newedep;
  }
}

comptonGenericDetectorSum::comptonGenericDetectorSum(const comptonGenericDetectorSum &right)
: G4VHit(right) {
  // copy constructor
  fDetID  = right.fDetID;
  fCopyID = right.fCopyID;
  fEdep   = right.fEdep;
  fSumByPID   = right.fSumByPID;
}

const comptonGenericDetectorSum& comptonGenericDetectorSum::operator=(const comptonGenericDetectorSum &right){
  (*this) = right;
  return *this;
}

G4int comptonGenericDetectorSum::operator==(const comptonGenericDetectorSum &right) const {
  return (this==&right) ? 1 : 0;
}
