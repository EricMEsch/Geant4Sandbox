#ifndef _OPTICAL_DETECTOR_HH_
#define _OPTICAL_DETECTOR_HH_

#include "G4RunManager.hh"
#include "G4VSensitiveDetector.hh"

#include "G4AnalysisManager.hh"

class OpticalDetector : public G4VSensitiveDetector {
public:
  OpticalDetector(G4String);
  ~OpticalDetector();

  bool ProcessHits(G4Step *step, G4TouchableHistory *history) override;
};

#endif
