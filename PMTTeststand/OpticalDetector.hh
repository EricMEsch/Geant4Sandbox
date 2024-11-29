#ifndef _OPTICAL_DETECTOR_HH_
#define _OPTICAL_DETECTOR_HH_

#include "G4RunManager.hh"
#include "G4VSensitiveDetector.hh"
#include "G4GenericMessenger.hh"
#include "G4AnalysisManager.hh"
#include <map>

class OpticalDetector : public G4VSensitiveDetector {
public:
  OpticalDetector(G4String);
  ~OpticalDetector();

  void Initialize(G4HCofThisEvent* hit_coll) override;
  bool ProcessHits(G4Step *step, G4TouchableHistory *history) override;
  void EndOfEvent(G4HCofThisEvent* hit_coll) override;
private:
  void DefineCommands();

  std::map<int, int> IntegralLightCounter; // key: detector copy number, value: light count

  std::unique_ptr<G4GenericMessenger> fGenericMessenger;
  bool fSurpressPhotonTimestamps = false;
  bool fSurpressIntegralLight = false;
};

#endif
