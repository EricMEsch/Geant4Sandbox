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

  // Build custom materials
  G4Element *H = nist->FindOrBuildElement("H");
  G4Element *C = nist->FindOrBuildElement("C");
  G4Element *O = nist->FindOrBuildElement("O");

  fMineralOil = new G4Material("MineralOil", 0.838 * g / cm3, 2);
  fMineralOil->AddElement(C, 1);
  fMineralOil->AddElement(H, 2);

  fPET = new G4Material("PET", 1.38 * g / cm3, 3);
  fPET->AddElement(C, 10);
  fPET->AddElement(H, 8);
  fPET->AddElement(O, 4);

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

  // Give Mineral Oil refractive index
  std::vector<G4double> RindexMineralOil = {
      1.467, 1.467}; // https://www.sigmaaldrich.com/DE/de/product/sial/161403
  G4MaterialPropertiesTable *mptMineralOil = new G4MaterialPropertiesTable();
  mptMineralOil->AddProperty("RINDEX", fenergyRindex, RindexMineralOil);
  // All plots for mineral oil are without units. But they all show transparency
  // until ~350nm
  mptMineralOil->AddProperty("ABSLENGTH", energyAbs, absorptionLengthPyrex);

  fMineralOil->SetMaterialPropertiesTable(mptMineralOil);

  // Give PET refractive index
  std::vector<G4double> RindexPET = {
      1.575, 1.575}; // https://de.delta-engineering.be/pet
  G4MaterialPropertiesTable *mptPET = new G4MaterialPropertiesTable();
  mptPET->AddProperty("RINDEX", fenergyRindex, RindexPET);
  // PET is not much different
  mptPET->AddProperty("ABSLENGTH", energyAbs, absorptionLengthPyrex);

  fPET->SetMaterialPropertiesTable(mptPET);
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
  double cathodeRadius = 127 * mm; // 254/2 mm
  double cathodeHeight = -80 * mm;

  double TotalWidth = fWindowWidth + fOilWidth + fPEWidth;
  double TotalHeight = cathodeHeight - TotalWidth;
  double Layer2Width = fWindowWidth + fOilWidth; // Just Window and Oil layer
  double OilHeight = cathodeHeight - Layer2Width;
  double WindowHeight = cathodeHeight - fWindowWidth;

  int numZPlanes1 = 11;
  // Can not use variable for length as C++ arrays have to be static
  double zPlane1[11], rInner1[11], rOuter1[11];
  double zPlane2[11], rInner2[11], rOuter2[11];
  double zPlane3[11], rInner3[11], rOuter3[11];
  double zPlane4[11], rInner4[11], rOuter4[11];

  // Type values manually as i do not trust floating point precision and i want
  // to compare the output with the old version
  double radiusFractions[11] = {1.0, 0.98, 0.95, 0.9,  0.85, 0.7,
                                0.6, 0.5,  0.4,  0.25, 0.0};
  double heightFractions[11] = {0.,  0.1, 0.2, 0.3, 0.4, 0.5,
                                0.6, 0.7, 0.8, 0.9, 1.0};

  for (int i = 0; i < numZPlanes1; ++i) {
    zPlane1[i] = cathodeHeight * heightFractions[i];
    rInner1[i] = 0;
    rOuter1[i] = cathodeRadius * radiusFractions[i];

    zPlane2[i] = TotalHeight * heightFractions[i];
    rInner2[i] = 0;
    rOuter2[i] = cathodeRadius * radiusFractions[i] + TotalWidth;

    zPlane3[i] = OilHeight * heightFractions[i];
    rInner3[i] = 0;
    rOuter3[i] = cathodeRadius * radiusFractions[i] + Layer2Width;

    zPlane4[i] = WindowHeight * heightFractions[i];
    rInner4[i] = 0;
    rOuter4[i] = cathodeRadius * radiusFractions[i] + fWindowWidth;
  }

  // PMT PET capsule
  auto *PMTPETsolid = new G4Polycone("PMTPET_solid", 0.0, CLHEP::twopi,
                                     numZPlanes1, zPlane2, rInner2, rOuter2);
  auto *PMTPETLogical = new G4LogicalVolume(PMTPETsolid, fPET, "PMTPET_log");
  new G4PVPlacement(0, G4ThreeVector(0., 0., 0.), PMTPETLogical, "PMTPET_phys",
                    logicWorld, false, 0, true);

  // G4Rotate3D rotateY(180*deg, G4ThreeVector(0,1,0));
  // G4Translate3D transX(G4ThreeVector(0.,0., -30.*cm));
  // G4Transform3D transformPMT = transX * rotateY;
  // new G4PVPlacement(transformPMT, PMTPETLogical, "PMTPET_phys",
  //                   logicWorld, false, 0, true);

  // Oil inside PMT
  auto *PMTOilsolid = new G4Polycone("PMTOil_solid", 0.0, CLHEP::twopi,
                                     numZPlanes1, zPlane3, rInner3, rOuter3);
  auto *PMTOilLogical =
      new G4LogicalVolume(PMTOilsolid, fMineralOil, "PMTOil_log");
  new G4PVPlacement(0, G4ThreeVector(0., 0., 0.), PMTOilLogical, "PMTOil_phys",
                    PMTPETLogical, false, 0, true);

  // Window inside PMT
  auto *PMTWindowsolid = new G4Polycone("PMTWindow_solid", 0.0, CLHEP::twopi,
                                        numZPlanes1, zPlane4, rInner4, rOuter4);
  auto *PMTWindowLogical =
      new G4LogicalVolume(PMTWindowsolid, fWindowMaterial, "PMTWindow_log");
  new G4PVPlacement(0, G4ThreeVector(0., 0., 0.), PMTWindowLogical,
                    "PMTWindow_phys", PMTOilLogical, false, 0, true);

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
      new G4Tubs("BackPlate_solid", 0.0 * cm, cathodeRadius * 1.2, 0.5 * cm,
                 0.0, CLHEP::twopi);
  auto *PMT_BackPlatelogical =
      new G4LogicalVolume(PMT_BackPlatesolid, fSteelmat, "BackPlate_log");
  auto *PMT_BackPlatephysical = new G4PVPlacement(
      nullptr, G4ThreeVector(0., 0., 0.5 * cm), PMT_BackPlatelogical,
      "BackPlate_phys", logicWorld, false, 0, true);

  auto *greyVisAtt = new G4VisAttributes(G4Colour::Grey());
  greyVisAtt->SetVisibility(true);
  auto *blackVisAtt = new G4VisAttributes(G4Colour::Black());
  blackVisAtt->SetVisibility(true);
  auto *brownVisAtt = new G4VisAttributes(G4Colour::Brown());
  brownVisAtt->SetVisibility(true);
  auto *blueVisAtt = new G4VisAttributes(G4Colour::Blue());
  blueVisAtt->SetVisibility(true);
  auto *cyanVisAtt = new G4VisAttributes(G4Colour::Cyan());
  cyanVisAtt->SetVisibility(true);

  logicWorld->SetVisAttributes(blackVisAtt);
  PMTOilLogical->SetVisAttributes(brownVisAtt);
  PMT_BackPlatelogical->SetVisAttributes(greyVisAtt);
  PMTWindowLogical->SetVisAttributes(blueVisAtt);
  PMTPETLogical->SetVisAttributes(cyanVisAtt);
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
  fGenericMessenger
      ->DeclareMethodWithUnit("SetOilWidth", "mm",
                              &DetectorConstruction::SetOilWidth)
      .SetGuidance("Set the width of the PMT Glass window")
      .SetParameterName("width", false)
      .SetStates(G4State_PreInit);
  fGenericMessenger
      ->DeclareMethodWithUnit("SetPETWidth", "mm",
                              &DetectorConstruction::SetPETWidth)
      .SetGuidance("Set the width of the PMT Glass window")
      .SetParameterName("width", false)
      .SetStates(G4State_PreInit);
}

//==============================================================================
