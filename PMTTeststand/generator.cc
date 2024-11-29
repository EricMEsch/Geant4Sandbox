#include "generator.hh"

MyPrimaryGenerator::MyPrimaryGenerator() {
  // Ready both guns
  fParticleGun = new G4ParticleGun(1);
  fParticleSource = new G4GeneralParticleSource();
}

//==============================================================================

MyPrimaryGenerator::~MyPrimaryGenerator() {
  delete fParticleGun;
  delete fParticleSource;
}

//==============================================================================

void MyPrimaryGenerator::GeneratePrimaries(G4Event *anEvent) {
  // Currently GPS selected
  fParticleSource->GeneratePrimaryVertex(anEvent);
}

//==============================================================================
