#include "OpticalDetector.hh"

#include "G4AnalysisManager.hh"
#include "G4EventManager.hh"
#include "G4OpticalPhoton.hh"
#include "G4SystemOfUnits.hh"

OpticalDetector::OpticalDetector(G4String name) : G4VSensitiveDetector(name) {
    DefineCommands();
}

//==============================================================================

OpticalDetector::~OpticalDetector() {}

//==============================================================================

// Invoked at the beginning of each event
void OpticalDetector::Initialize(G4HCofThisEvent *hit_coll) {
  IntegralLightCounter.clear();
}

//==============================================================================

G4bool OpticalDetector::ProcessHits(G4Step *step, G4TouchableHistory *ROhist) {
  // Is Optical?
  // ( •_•)
  // >⌐■--■⌐<
  auto particle = step->GetTrack()->GetDefinition();
  if (particle != G4OpticalPhoton::OpticalPhotonDefinition())
    return false;

  // Not relevant for now, use as crosscheck
  auto touchable = step->GetPostStepPoint()->GetTouchableHandle();
  const auto pv_name = touchable->GetVolume()->GetName();
  const auto pv_copynr = touchable->GetCopyNumber();
  if (pv_name != "PMT_phys") {
    G4cerr << "Warning: Photon detected leaving PMT??" << G4endl;
    G4cerr << "No idea what G4 is doing. Skipping Photon." << G4endl;
    return false;
  }

  if(!fSurpressPhotonTimestamps) {
    auto event = G4EventManager::GetEventManager()->GetNonconstCurrentEvent();
    auto photon_wavelength =
        CLHEP::c_light * CLHEP::h_Planck / step->GetTotalEnergyDeposit();
    auto global_time = step->GetPostStepPoint()->GetGlobalTime();

    const auto ana_man = G4AnalysisManager::Instance();
    int col_id = 0;
    ana_man->FillNtupleIColumn(0, col_id++, event->GetEventID());
    ana_man->FillNtupleIColumn(0, col_id++, pv_copynr);
    ana_man->FillNtupleDColumn(0, col_id++, photon_wavelength / nm);
    ana_man->FillNtupleDColumn(0, col_id++, global_time / ns);
    // NOTE: must be called here for hit-oriented output
    ana_man->AddNtupleRow(0);
  }
  IntegralLightCounter[pv_copynr]++;
  return true; // return is not used by geant4 kernel, so doesn't matter
}

//==============================================================================

void OpticalDetector::EndOfEvent(G4HCofThisEvent *hit_coll)
{
  if (!fSurpressIntegralLight) {
    auto event = G4EventManager::GetEventManager()->GetNonconstCurrentEvent();
    const auto ana_man = G4AnalysisManager::Instance();
    for (const auto& [detector_id, light_count] : IntegralLightCounter) {
      if (light_count > 0) {
        int col_id = 0;
        ana_man->FillNtupleIColumn(1, col_id++, event->GetEventID());
        ana_man->FillNtupleIColumn(1, col_id++, detector_id);
        ana_man->FillNtupleIColumn(1, col_id++, light_count);
        ana_man->AddNtupleRow(1);
      }
    }
  }
}

//==============================================================================

void OpticalDetector::DefineCommands()
{
    fGenericMessenger = std::make_unique<G4GenericMessenger>(
      this, "/Sandbox/Output/", "Control of the output");

  fGenericMessenger->DeclareProperty("DisablePhotonTimeStamps", fSurpressPhotonTimestamps)
      .SetGuidance("Disable storing photon timestamps")
      .SetStates(G4State_Idle);
  fGenericMessenger->DeclareProperty("DisableIntegralLight", fSurpressIntegralLight)
      .SetGuidance("Disable storing integral light per event")
      .SetStates(G4State_Idle);
}

//==============================================================================
