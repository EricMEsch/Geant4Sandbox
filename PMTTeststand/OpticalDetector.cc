#include "OpticalDetector.hh"

#include "G4AnalysisManager.hh"
#include "G4EventManager.hh"
#include "G4OpticalPhoton.hh"
#include "G4SystemOfUnits.hh"

OpticalDetector::OpticalDetector(G4String name) : G4VSensitiveDetector(name) {}

//==============================================================================

OpticalDetector::~OpticalDetector() {}

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
    G4Exception("OpticalDetector::ProcessHits", "Custom code", FatalException,
                "Photon detected, but not in PMT");
  }

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
  return true; // return is not used by geant4 kernel, so doesn't matter
}

//==============================================================================
