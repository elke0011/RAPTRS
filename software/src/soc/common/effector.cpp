/*
Copyright (c) 2016 - 2019 Regents of the University of Minnesota and Bolder Flight Systems Inc.
MIT License; See LICENSE.md for complete details
Author: Brian Taylor
*/

#include "effector.h"

/*
Method to configure effectors and register inputs with definition tree. A typical JSON configuration would be:

"Effectors": [
  { "Type": "Sbus",
    "Input": "/Control/Elevator",
    "Channel": 2,
    "Calibration": [40.975,991.5]},
    "Safed-Command":
]

Where:
   * Type can be Sbus, Pwm, or Motor specifying whether the effector is a SBUS or PWM servo or a motor.
     Motors are considered PWM servos, but include throttle safety logic.
   * Input specifies the path name to the input command.
   * Channel is the channel number of the effector.
   * Calibration is a vector of polynomial coefficients listed in descending order to convert the angle
     or power command to an SBUS (172-1811) or PWM (1000-2000 us) output value.
   * Safed command is an optional value specifying the command to output for a motor when the throttle is
     safed. This is the input command to the calibration, not the value sent to the motor (i.e. a value of
     -1 instead of 1000 us).
*/

/*
   Note: effector.cpp needs to know about physical hardware layout
   (i.e. where actuators are connected down stream and to which nodes.
   Preferably this knowledge should live inside fmu.cpp.  The flight
   code shouldn't need to care about the physical layout of the
   effectors.

   This code does definition tree map lookups every frame.  This way
   when the /Control/ aliases get remade, this module isn't left
   pointing to the old Element records.
*/

void AircraftEffectors::Configure(const rapidjson::Value& Config) {

  ElementPtr ele;
  std::string eleKey;

  assert(Config.IsArray());
  for (size_t i=0; i < Config.Size(); i++) {
    const rapidjson::Value& Effector = Config[i];
    if (Effector.HasMember("Type")) {
      if (Effector["Type"] == "Node") {
        if (Effector.HasMember("Effectors")) {
          assert(Effector["Effectors"].IsArray());
          for (auto &NodeEffector : Effector["Effectors"].GetArray()) {
            LoadInput(NodeEffector, "", "Input", &ele, &eleKey);
            EffNodeVec_.push_back(ele);
            EffKeyVec_.push_back(eleKey);
          }
        } else {
          throw std::runtime_error(std::string("ERROR")+RootPath_+std::string(": Node effectors not specified in configuration."));
        }
      } else {
        LoadInput(Effector, "", "Input", &ele, &eleKey);
        EffNodeVec_.push_back(ele);
        EffKeyVec_.push_back(eleKey);
      }
    } else {
      throw std::runtime_error(std::string("ERROR")+RootPath_+std::string(": Type not specified in configuration."));
    }
  }
  Configured_ = true;
}

/* Run method for effectors, dereferences the inputs and returns a vector of effector commands to send to the FMU */
std::vector<float> AircraftEffectors::Run() {
  std::vector<float> Commands;
  Commands.resize(EffNodeVec_.size());
  for (size_t i=0; i < EffNodeVec_.size(); i++) {
    Commands[i] = EffNodeVec_[i]->getFloat();
  }
  return Commands;
}

/* Returns whether the class has been configured */
bool AircraftEffectors::Configured() {
  return Configured_;
}

/* Get Keys */
std::vector<std::string> AircraftEffectors::GetKeys() {
  return EffKeyVec_;
}
