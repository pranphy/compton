#ifndef comptonUserTrackInformation_h_
#define comptonUserTrackInformation_h_

#include "G4StepStatus.hh"
#include "G4VUserTrackInformation.hh"

class comptonUserTrackInformation : public G4VUserTrackInformation
{
  public:
    comptonUserTrackInformation() { fStepStatus = fUndefined; };
    virtual ~comptonUserTrackInformation() { };
    G4StepStatus GetStepStatus() const { return fStepStatus; };
    void SetStepStatus(G4StepStatus stepstatus) { fStepStatus = stepstatus; };
  private:
    G4StepStatus fStepStatus;
};

#endif /* comptonUserTrackInformation_h_ */
