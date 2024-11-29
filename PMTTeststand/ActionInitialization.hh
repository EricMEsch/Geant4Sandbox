#ifndef ACTION_INITIALIZATION_HH
#define ACTION_INITIALIZATION_HH

#include <G4VUserActionInitialization.hh>
#include <string>

class ActionInitialization : public G4VUserActionInitialization {
public:
  ActionInitialization(std::string outputName);
  ~ActionInitialization();
  void Build() const override;

  void BuildForMaster() const override;

private:
  std::string fOutputName;
};

#endif
