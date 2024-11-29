#ifndef _DETECTOR_CONSTRUCTION_HH_
#define _DETECTOR_CONSTRUCTION_HH_

#include "G4Box.hh"
#include "G4GenericMessenger.hh"
#include "G4LogicalVolume.hh"
#include "G4NistManager.hh"
#include "G4PVPlacement.hh"
#include "G4SystemOfUnits.hh"
#include "G4VPhysicalVolume.hh"
#include "G4VUserDetectorConstruction.hh"

#include "OpticalDetector.hh"

class DetectorConstruction : public G4VUserDetectorConstruction {
public:
  DetectorConstruction();
  ~DetectorConstruction();

  virtual G4VPhysicalVolume *Construct();

private:
  void defineMaterials();
  void defineVolumes();
  void defineBoundaries();

  void ConstructSDandField() override;

  void DefineCommands();

  void SetWindowWidth(double width) { fWindowWidth = width; }

  G4Material *fH2O, *fWindowMaterial, *fAirmat, *fVac, *fSteelmat;

  G4LogicalVolume *fPMTLogical;

  G4VPhysicalVolume *fphysWorld;
  double fWindowWidth = 4.0 * mm;
  std::unique_ptr<G4GenericMessenger> fGenericMessenger;
};

#endif
