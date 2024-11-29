#ifndef STEPPINGACTION_HH
#define STEPPINGACTION_HH

#include "EventAction.hh"
#include <G4UserSteppingAction.hh>

class RunAction;

class SteppingAction : public G4UserSteppingAction {
public:
  //! constructor
  SteppingAction(RunAction *, EventAction *);

  void UserSteppingAction(const G4Step *) override;

private:
  RunAction *fRunAction;
  EventAction *fEventAction;
};

#endif
