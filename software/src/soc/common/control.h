/*
Copyright (c) 2016 - 2019 Regents of the University of Minnesota and Bolder Flight Systems Inc.
MIT License; See LICENSE.md for complete details
Author: Brian Taylor and Chris Regan
*/

#pragma once

#include "configuration.h"
#include "definition-tree2.h"

#include "generic-function.h"
#include "general-functions.h"
#include "control-functions.h"
#include "allocation-functions.h"
#include "filter-functions.h"

#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <termios.h>
#include <stdint.h>
#include <exception>
#include <stdexcept>
#include <vector>
#include <cstring>
#include <memory>

/* Class to manage control laws
Example JSON configuration:

"Control": {
    "Fmu": "Group1",
    "Baseline": ["Group2", "Group3", "Group3"], // Selected is always Armed or Engaged
    "Test": ["Group2", "Group3", "Group4", "Group5"], // Only one is either Armed or Engaged by Command

    "GroupDef": {
      "Group1": {
        "Level1": "Controller1"},
      "Group2": {
        "Level1": "Controller2", "Level2": "Controller3"}
    },

    "ControlDef": {
      "Controller1": [{Component1}, {Component2}, ... ],
      "Controller2": [... ],
      "Controller3": [... ]
    }
}


Where:
   * FMU is a vector of control law group names for the Flight Management Unit to use.
   * SOC is a vector of control law group names for the SOC to use.
   * Control law goups are named as arrays. Each array contains an object for each control
     law level starting with the outermost level and working to the innermost. Each level
     is named for easy reference by the excitation system. Components for each control
     level are objects configuring the specific control law.
*/

// Wrapper to contain all the Components defined in a Controller
class ComponentWrapper {
  public:
     void Configure (std::string ControllerPath, const rapidjson::Value& Controller);
     void Run (GenericFunction::Mode mode);
   private:
     std::vector<std::shared_ptr<GenericFunction>> ComponentVec_;
};

class ControlSystem {
  public:
     // Vector listing the Groups within a set.
    typedef std::vector<std::string> SetVec;

    // Create a Map of Controller Groups
    struct GroupStruct {
      std::string Level;
      std::string Controller;
    } ;
    typedef std::vector<GroupStruct> GroupVec;
    typedef std::map<std::string, GroupVec> GroupMap;

    // Create a Map of Controller Objects (Wrapped in Component Wrapper)
    typedef std::shared_ptr<ComponentWrapper> ComponentWrapperPtr ;
    typedef std::map<std::string, ComponentWrapperPtr> ControlMap;

    // Containers for deftree nodes
    typedef std::vector<ElementPtr> NodeVec;
    struct NodeStruct {
      NodeVec Controller; // Vector of Nodes at Controller path (/Control/Baseline/<Controller1>/<Elem1>)
      NodeVec Set;        // Vector of Nodes at Set path (/Control/Baseline/<Elem1>)
      NodeVec Root;       // Vector of Nodes at Root path (/Control/<Elem1>)
    } ;
    typedef std::map<std::string, NodeStruct> NodeMap; // Key is <Controller1>

    // Configuration Methods
    void Configure(const rapidjson::Value& Config);
    void ConfigureSet(std::string SetPath, const rapidjson::Value& SetDef, const rapidjson::Value& GroupDef, const rapidjson::Value& ControlDef, GroupMap *SetGroupMap, ControlMap *SetControlMap, NodeMap *SetNodeMap);
    void ConfigureEffectors(std::vector<std::string> EffKeyVec);

    // Baseline Controller
    std::string GetBaseline();
    void SetBaseline(std::string GroupSel);
    void RunBaseline(GenericFunction::Mode mode);
    void EffectorBaseline(GenericFunction::Mode mode);

    // Test Controller
    std::string GetTest();
    void SetTest(std::string TestGroupSel);
    void RunTest(GenericFunction::Mode mode);
    void EffectorTest(GenericFunction::Mode mode);

    std::vector<std::string> GetTestLevels();
    std::string GetLevel();
    void SetLevel(std::string TestLevelSel);

  private:
    std::string RootPath_ = "/Control";
    std::string BaselinePath_, TestPath_;
    std::string EngagedSet_ = "Fmu";
    std::string ArmedSet_ = "Fmu";
    GroupMap BaselineGroupMap_, TestGroupMap_;
    ControlMap BaselineControlMap_, TestControlMap_;
    std::string BaselineGroupSel_;
    std::string TestGroupSel_;
    std::string TestLevelSel_;

    NodeVec BaselineEffNodeVec_, TestEffNodeVec_, EffNodeVec_;
    NodeMap BaselineNodeMap_, TestNodeMap_;
};
