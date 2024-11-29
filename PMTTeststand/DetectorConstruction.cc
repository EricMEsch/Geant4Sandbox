#include "DetectorConstruction.hh"

#include "G4Box.hh"
#include "G4Colour.hh"
#include "G4Cons.hh"
#include "G4GDMLParser.hh"
#include "G4GeometryManager.hh"
#include "G4LogicalVolume.hh"
#include "G4LogicalVolumeStore.hh"
#include "G4Material.hh"
#include "G4NistManager.hh"
#include "G4PVPlacement.hh"
#include "G4PVReplica.hh"
#include "G4PhysicalVolumeStore.hh"
#include "G4PhysicsOrderedFreeVector.hh"
#include "G4SolidStore.hh"
#include "G4Tubs.hh"
#include "G4VisAttributes.hh"

#include "G4PhysicalConstants.hh"
#include "G4SystemOfUnits.hh"

#include "G4LogicalBorderSurface.hh"
#include "G4LogicalSkinSurface.hh"
#include "G4OpticalSurface.hh"
#include "G4SDManager.hh"

DetectorConstruction::DetectorConstruction() // Constructor
{
  DefineCommands();
}

//==============================================================================

DetectorConstruction::~DetectorConstruction() // Destructor
{}

//==============================================================================

// Creates Physical Volume, returns Pointer to said volume
G4VPhysicalVolume *DetectorConstruction::Construct() {
  defineMaterials();
  defineVolumes();
  defineBoundaries();
  return fphysWorld;
}

//==============================================================================

// Defines Materials used in the simulation
void DetectorConstruction::defineMaterials() {
  G4NistManager *nist = G4NistManager::Instance();
  fWindowMaterial = nist->FindOrBuildMaterial("G4_Pyrex_Glass");
  if (!fWindowMaterial) {
    G4Exception("DetectorConstruction::defineMaterials()", "Custom Code",
                FatalException, "G4_Pyrex_Glass material not found.");
  }
  /* Pyrex Glass according to NIST:
   * 4% B, 53% O, 2.8% Na, 1.1% Al, 37.7% Si, 0.3% K
   * https://geant4-userdoc.web.cern.ch/UsersGuides/ForApplicationDeveloper/html/Appendix/materialNames.html
   */

  fVac = nist->FindOrBuildMaterial("G4_Galactic");
  fSteelmat = nist->FindOrBuildMaterial("G4_STAINLESS-STEEL");
  fAirmat = nist->FindOrBuildMaterial("G4_AIR");

  //==============================================================================

  // add optical properties of materials
  std::vector<G4double> fenergyRindex = {
      1.239841939 * eV / 0.6,
      1.239841939 * eV / 0.1}; // Convert energie from wavelength to eV
  // Best i came up with, without any knowledge of the material and 10s research
  std::vector<G4double> RindexPyrex = {
      1.474,
      1.474}; // https://www.flinnsci.com/api/library/Download/80837f7444274b01b0a75f2639c2d284

  std::vector<G4double> energyAbs = {
      1.239841939 * eV / 0.6, 1.239841939 * eV / 0.35, 1.239841939 * eV / 0.28,
      1.239841939 * eV / 0.1};
  std::vector<G4double> absorptionLengthPyrex = {
      10.0 * cm, 10.0 * cm, 0.1 * mm,
      0.1 *
          mm}; // Absorption length, adjusted to
               // https://www.researchgate.net/figure/Optical-transmittance-of-pyrex-and-quartz-spectrometric-cells-and-015-M-ferrioxalate_fig2_228690447

  G4MaterialPropertiesTable *mptPyrex = new G4MaterialPropertiesTable();
  mptPyrex->AddProperty("RINDEX", fenergyRindex, RindexPyrex);
  mptPyrex->AddProperty("ABSLENGTH", energyAbs, absorptionLengthPyrex);

  fWindowMaterial->SetMaterialPropertiesTable(mptPyrex);

  // Give Air refractive index, so that light can travel through it
  std::vector<G4double> RindexAir = {1.0, 1.0};
  G4MaterialPropertiesTable *mptAir = new G4MaterialPropertiesTable();
  mptAir->AddProperty("RINDEX", fenergyRindex, RindexAir);

  fAirmat->SetMaterialPropertiesTable(mptAir);
}

//==============================================================================

void DetectorConstruction::defineVolumes() {
  // World boundaries. Make it resemble the dark box
  // I measured 38x42x100 cm
  G4double xWorld = 0.19 * m; // Remember these are half values
  G4double yWorld = 0.21 * m;
  G4double zWorld = 0.5 * m;

  // Create World Volume
  auto *solidWorld = new G4Box("World_solid", xWorld, yWorld, zWorld);
  auto *logicWorld = new G4LogicalVolume(solidWorld, fAirmat, "World_log");
  fphysWorld = new G4PVPlacement(0, G4ThreeVector(0., 0., 0.), logicWorld,
                                 "World_phys", 0, false, 0, true);

  // Build a PMT
  G4double realRadius = 127 * mm; // 254/2 mm
  G4double realHeight = -80 * mm;
  G4int numZPlanes1 = 11;
  G4double zPlane1[] = {0,
                        realHeight * 0.1,
                        realHeight * 0.2,
                        realHeight * 0.3,
                        realHeight * 0.4,
                        realHeight * 0.5,
                        realHeight * 0.6,
                        realHeight * 0.7,
                        realHeight * 0.8,
                        realHeight * 0.9,
                        realHeight};
  G4double rInner1[] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
  G4double rOuter1[] = {realRadius,
                        realRadius * 0.98,
                        realRadius * 0.95,
                        realRadius * 0.9,
                        realRadius * 0.85,
                        realRadius * 0.7,
                        realRadius * 0.6,
                        realRadius * 0.5,
                        realRadius * 0.4,
                        realRadius * 0.25,
                        0};

  G4double realHeight2 = realHeight - fWindowWidth;
  // The PMT window
  G4double zPlane2[] = {0,
                        realHeight2 * 0.1,
                        realHeight2 * 0.2,
                        realHeight2 * 0.3,
                        realHeight2 * 0.4,
                        realHeight2 * 0.5,
                        realHeight2 * 0.6,
                        realHeight2 * 0.7,
                        realHeight2 * 0.8,
                        realHeight2 * 0.9,
                        realHeight2};
  G4double rInner2[] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
  G4double rOuter2[] = {realRadius + fWindowWidth,
                        realRadius * 0.98 + fWindowWidth,
                        realRadius * 0.95 + fWindowWidth,
                        realRadius * 0.9 + fWindowWidth,
                        realRadius * 0.85 + fWindowWidth,
                        realRadius * 0.7 + fWindowWidth,
                        realRadius * 0.6 + fWindowWidth,
                        realRadius * 0.5 + fWindowWidth,
                        realRadius * 0.4 + fWindowWidth,
                        realRadius * 0.25 + fWindowWidth,
                        0};

  auto *PMTWindowsolid = new G4Polycone("PMTWindow_solid", 0.0, CLHEP::twopi,
                                        numZPlanes1, zPlane2, rInner2, rOuter2);
  auto *PMTWindowLogical =
      new G4LogicalVolume(PMTWindowsolid, fWindowMaterial, "PMTWindow_log");
  new G4PVPlacement(0, G4ThreeVector(0., 0., 0.), PMTWindowLogical,
                    "PMTWindow_phys", logicWorld, false, 0, true);

  // The sensitive PMT
  auto *PMTsolid = new G4Polycone("PMT_solid", 0.0, CLHEP::twopi, numZPlanes1,
                                  zPlane1, rInner1, rOuter1);
  fPMTLogical = new G4LogicalVolume(
      PMTsolid, fVac, "PMT_log"); // Material shouldn't matter, as the optical
                                  // surface is responsible for the efficiency
  auto *PMTPhysical =
      new G4PVPlacement(0, G4ThreeVector(0., 0., 0.), fPMTLogical, "PMT_phys",
                        PMTWindowLogical, false, 0, true);

  // BackPlate can't be a daughter of PMT due to the polycone behaving weird. So
  // we place it in the World The visualisation might look like there is a hole
  // between them, but the tracking shows that there is not
  auto *PMT_BackPlatesolid =
      new G4Tubs("BackPlate_solid", 0.0 * cm, realRadius * 1.2, 0.5 * cm, 0.0,
                 CLHEP::twopi);
  auto *PMT_BackPlatelogical =
      new G4LogicalVolume(PMT_BackPlatesolid, fSteelmat, "BackPlate_log");
  auto *PMT_BackPlatephysical = new G4PVPlacement(
      nullptr, G4ThreeVector(0., 0., 0.5 * cm), PMT_BackPlatelogical,
      "BackPlate_phys", logicWorld, false, 0, true);

  auto *greyVisAtt = new G4VisAttributes(G4Colour::Grey());
  greyVisAtt->SetVisibility(true);
  auto *worldVisAtt = new G4VisAttributes(G4Colour::White());
  worldVisAtt->SetVisibility(false);
  auto *PMTVisAtt = new G4VisAttributes(G4Colour::Brown());
  PMTVisAtt->SetVisibility(true);
  auto *BlueVisAtt = new G4VisAttributes(G4Colour::Blue());
  BlueVisAtt->SetVisibility(true);

  logicWorld->SetVisAttributes(worldVisAtt);
  fPMTLogical->SetVisAttributes(PMTVisAtt);
  PMT_BackPlatelogical->SetVisAttributes(greyVisAtt);
  PMTWindowLogical->SetVisAttributes(BlueVisAtt);
}

//==============================================================================

// Define the PMT detection boundary
void DetectorConstruction::defineBoundaries() {
  // PMT Boundary for detection
  G4OpticalSurface *PMTsurface = new G4OpticalSurface("PMTSurface");
  G4LogicalSkinSurface *PMTSkinSurface =
      new G4LogicalSkinSurface("PMTSkinSurface", fPMTLogical, PMTsurface);
  PMTsurface->SetType(dielectric_metal);
  PMTsurface->SetModel(glisur);
  PMTsurface->SetFinish(polished);

  G4MaterialPropertiesTable *PMT_MPT = new G4MaterialPropertiesTable();
  G4PhysicsOrderedFreeVector *QuantumEfficiency =
      new G4PhysicsOrderedFreeVector;

  // Read in the correct quantum efficiency and add to vectors
  std::ifstream datafile;
  datafile.open("../data/R7081_QEWhitespace.csv");
  while (datafile.is_open()) {
    G4double wlen, queff;

    datafile >> wlen >> queff;

    // convert the wavelength (supposed to be in nm) to energy
    QuantumEfficiency->InsertValues(1239.841939 * eV / wlen, queff / 100.);
    if (datafile.eof())
      break;
  }
  datafile.close();
  // Overwrite efficiency with new read in efficiency
  PMT_MPT->AddProperty("EFFICIENCY", QuantumEfficiency);
  std::vector<G4double> fenergyRindex = {1.239841939 * eV / 0.6,
                                         1.239841939 * eV / 0.1};
  std::vector<G4double> PMT_reflectivity = {0., 0.};
  PMT_MPT->AddProperty("REFLECTIVITY", fenergyRindex, PMT_reflectivity);
  PMTsurface->SetMaterialPropertiesTable(PMT_MPT);
}

//==============================================================================

void DetectorConstruction::ConstructSDandField() {
  auto sd_man = G4SDManager::GetSDMpointer();
  OpticalDetector *sensDet = new OpticalDetector("OpticalDetector");
  sd_man->AddNewDetector(sensDet);

  this->SetSensitiveDetector(fPMTLogical, sensDet);
}

//==============================================================================

void DetectorConstruction::DefineCommands() {
  fGenericMessenger = std::make_unique<G4GenericMessenger>(
      this, "/Sandbox/Construction/", "Control of the Detector Construction");

  fGenericMessenger
      ->DeclareMethodWithUnit("SetWindowWidth", "mm",
                              &DetectorConstruction::SetWindowWidth)
      .SetGuidance("Set the width of the PMT Glass window")
      .SetParameterName("width", false)
      .SetStates(G4State_PreInit);
}

//==============================================================================
