#include "EventAction.hh"

#include "G4AnalysisManager.hh"
#include "G4RunManager.hh"
#include <G4Event.hh>
#include <G4SDManager.hh>
#include <G4SystemOfUnits.hh>
#include <G4THitsMap.hh>

EventAction::EventAction(RunAction *RunAction) : fRunAction(RunAction) {}

//==============================================================================

void EventAction::BeginOfEventAction(const G4Event *event) {
  // Do a fancy progress bar (this might slow down the simulation if many short
  // events are computed)
  auto g4manager = G4RunManager::GetRunManager();
  auto total_events = g4manager->GetNumberOfEventsToBeProcessed();
  auto event_id = event->GetEventID() + 1;

  if (total_events > 100) {
    auto print_modulo = total_events / 100;

    if (event_id % print_modulo == 0) {
      G4cout << "Processing event nr. " << event_id << G4endl;

      double progress = (double)event_id / total_events * 100.0;
      int barWidth = 70;
      G4cout << "[";
      int pos = barWidth * progress / 100.0;
      for (int i = 0; i < barWidth; ++i) {
        if (i < pos)
          G4cout << "=";
        else if (i == pos)
          G4cout << ">";
        else
          G4cout << " ";
      }
      G4cout << "] " << int(progress) << " %\r";
      G4cout.flush();
    }
  }
}

//==============================================================================

void EventAction::EndOfEventAction(const G4Event *event) {}

//==============================================================================
