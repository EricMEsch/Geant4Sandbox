#ifndef EVENTACTION_HH
#define EVENTACTION_HH

#include <G4UserEventAction.hh>
#include <globals.hh>

#include "RunAction.hh"

class EventAction : public G4UserEventAction {
public:
  EventAction(RunAction *);
  virtual void BeginOfEventAction(const G4Event *);
  void EndOfEventAction(const G4Event *event) override;

private:
  RunAction *fRunAction;
};

#endif
