#include "ActionInitialization.hh"
#include "generator.hh"

#include "EventAction.hh"
#include "RunAction.hh"
#include "SteppingAction.hh"

ActionInitialization::ActionInitialization(std::string outputName)
    : fOutputName(outputName) {}

//==============================================================================

ActionInitialization::~ActionInitialization() {}

//==============================================================================

void ActionInitialization::Build() const {
  // Set up generator
  SetUserAction(new MyPrimaryGenerator());
  // Set up output
  RunAction *theRunAction = new RunAction(fOutputName);
  SetUserAction(theRunAction);
  EventAction *theEventAction = new EventAction(theRunAction);
  SetUserAction(theEventAction);
  SetUserAction(new SteppingAction(theRunAction, theEventAction));
}

//==============================================================================

void ActionInitialization::BuildForMaster() const {
  // Only relevant in MT mode. MT COMPATIBILITY NOT TESTED
  SetUserAction(new RunAction(fOutputName));
}

//==============================================================================
