#include "SteppingAction.hh"
#include "EventAction.hh"
#include "RunAction.hh"

#include "G4Geantino.hh"
#include "G4Step.hh"

SteppingAction::SteppingAction(RunAction *runAction, EventAction *EventAction)
    : fRunAction(runAction), fEventAction(EventAction) {}

//==============================================================================

void SteppingAction::UserSteppingAction(const G4Step *aStep) {}

//==============================================================================
