#include "RunAction.hh"
#include "G4SystemOfUnits.hh"

RunAction::RunAction(std::string outputName) : fOutputName(outputName) {
  G4AnalysisManager *man = G4AnalysisManager::Instance();
  man->CreateNtuple("PhotonHits", "PhotonHits");
  man->CreateNtupleIColumn("evtID");
  man->CreateNtupleIColumn("det_uid"); // In case there are multiple PMTs
  man->CreateNtupleDColumn("wavelength_in_nm");
  man->CreateNtupleDColumn("time_in_ns");
  man->FinishNtuple(0);

  man->CreateNtuple("TotalHits", "TotalHits");
  man->CreateNtupleIColumn("evtID");
  man->CreateNtupleIColumn("det_uid"); // In case there are multiple PMTs
  man->CreateNtupleIColumn("TotalHits");
  man->FinishNtuple(1);
}

//==============================================================================

RunAction::~RunAction() {}

//==============================================================================

void RunAction::BeginOfRunAction(const G4Run *run) {
  G4AnalysisManager *man = G4AnalysisManager::Instance();

  // Use dynamic output name. If no extension specified default to .root
  G4int runID = run->GetRunID();
  std::stringstream strRunID;
  strRunID << runID;
  size_t pos = fOutputName.find_last_of(".");
  std::string baseName =
      (pos == std::string::npos) ? fOutputName : fOutputName.substr(0, pos);
  std::string extension =
      (pos == std::string::npos) ? ".root" : fOutputName.substr(pos);
  if (pos == std::string::npos) {
    G4cout << "Warning: No file extension found. Defaulting to .root" << G4endl;
  }
  std::string dynamicOutputName = baseName + strRunID.str() + extension;
  man->OpenFile(dynamicOutputName);
}

//==============================================================================

void RunAction::EndOfRunAction(const G4Run *run) {
  // retrieve the number of events produced in the run
  G4int nofEvents = run->GetNumberOfEvent();
  if (nofEvents == 0)
    return;
  G4AnalysisManager *man = G4AnalysisManager::Instance();

  man->Write();
  man->CloseFile();
}

//==============================================================================
