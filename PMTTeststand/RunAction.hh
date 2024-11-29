#ifndef RUNACTION_HH
#define RUNACTION_HH

#include "G4AnalysisManager.hh"
#include "G4ParticleDefinition.hh"
#include "G4Run.hh"
#include "G4UserRunAction.hh"

class RunAction : public G4UserRunAction {
public:
  //! constructor
  RunAction(std::string outputName);

  //! destructor
  ~RunAction();

  //! Main interface
  void BeginOfRunAction(const G4Run *);
  void EndOfRunAction(const G4Run *);

private:
  std::string fOutputName;
};

#endif
