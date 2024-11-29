#include <iostream>

#include "G4MTRunManager.hh"
#include "G4OpticalParameters.hh"
#include "G4OpticalPhysics.hh"
#include "G4RunManager.hh"
#include "G4UIExecutive.hh"
#include "G4UImanager.hh"
#include "G4VisExecutive.hh"
#include "G4VisManager.hh"
#include "Shielding.hh"

#include "ActionInitialization.hh"
#include "DetectorConstruction.hh"

#include "CLI11.hpp"

int main(int argc, char **argv) {
  CLI::App app{"PMT Teststand simulations"};
  int nthreads = 1;
  std::string macroName;
  std::string outputName = "output.root";

  app.add_option("-m,--macro", macroName,
                 "<Geant4 macro filename> Default: None");
  app.add_option("-t, --nthreads", nthreads,
                 "<number of threads to use> Default: 1");
  app.add_option("-o, --output", outputName,
                 "<Output filename> Default: 'output.root'");

  CLI11_PARSE(app, argc, argv);

  // Allow Multi-threading if available, although not tested
  G4RunManager *runManager = nullptr;

#ifdef G4MULTITHREADED

  if (nthreads > 1) {
    nthreads = std::min(
        nthreads, G4Threading::G4GetNumberOfCores()); // limit thread number to
                                                      // max on machine

    runManager = new G4MTRunManager();
    G4cout << "      ********* Run Manager constructed in MT mode: " << nthreads
           << " threads ***** " << G4endl;
    G4cout << "      ********* WARNING: MT Compatibility is not tested! ***** "
           << G4endl;
    runManager->SetNumberOfThreads(nthreads);
  } else {
    runManager = new G4RunManager();
    G4cout << "      ********** Run Manager constructed in sequential mode "
              "************ "
           << G4endl;
  }

#else
  runManager = new G4RunManager();
  G4cout << "      ********** Run Manager constructed in sequential mode "
            "************ "
         << G4endl;
#endif

  /* Initialize all custom implemented stuff*/
  runManager->SetUserInitialization(new DetectorConstruction());
  // Basic physics list, should include all relevant processes
  G4VModularPhysicsList *physics = new Shielding();
  physics->RegisterPhysics(new G4OpticalPhysics());
  G4OpticalParameters *op_par = G4OpticalParameters::Instance();
  op_par->SetBoundaryInvokeSD(true);
  runManager->SetUserInitialization(physics);
  // Our custom action initialization
  runManager->SetUserInitialization(new ActionInitialization(outputName));

  G4UIExecutive *ui = 0;

  /*only generate graphic output if no macro specified*/
  if (macroName.empty()) {
    ui = new G4UIExecutive(argc, argv);
  }

  G4VisManager *visManager = new G4VisExecutive();
  visManager->Initialize();

  G4UImanager *UImanager = G4UImanager::GetUIpointer();

  if (ui) {
    /*Load visualization macro file*/
    UImanager->ApplyCommand("/control/execute vis.mac");
    ui->SessionStart();
  } else {
    G4String command = "/control/execute ";
    UImanager->ApplyCommand(command + macroName);
  }

  return 0;
}
