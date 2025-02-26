/*
Copyright (c) 2016 - 2019 Regents of the University of Minnesota and Bolder Flight Systems Inc.
MIT License; See LICENSE.md for complete details
Author: Brian Taylor, Chris Regan
*/

#pragma once

#include "hardware-defs.h"
#include "configuration.h"
#include "generic-function.h"
#include "general-functions.h"
#include "filter-functions.h"
#include "airdata-functions.h"
#include "ins-functions.h"
#include "power.h"

#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <termios.h>
#include <stdint.h>
#include <iostream>
#include <exception>
#include <stdexcept>
#include <vector>
#include <cstring>
#include <memory>

/*
  "Sensor-Processing": {
    "Fmu": "System1",
    "Baseline": "System1",
    "Test": ["System1"],

    "Def": {
      "System1": [
        {Component1},
        {Component2},
        {Component3},
        {Component4}
      ]
    }
  }


  Where:
     * FMU is a System name coorsponding to a SensorProcessing System for the Flight Management Unit to use.
     * Baseline is a System name coorsponding to a SensorProcessing System to run as the Baseline system.
        The Baseline system always runs in Armed or Engaged mode.
     * Test is a vector of System names coorsponding to SensorProcessing Systems to run as the Test systems.
        The Baseline system will be excluded from this group if included.
        Test System can run in any mode, but only one system can be Armed or Engaged at a time.

*/

// Wrapper to contain all the Components defined in a SensorProc Definition
class SystemWrapper {
  public:
     void Configure (std::string SystemName, const rapidjson::Value& System);
     void Initialize ();
     bool Initialized ();
     void Run (GenericFunction::Mode mode);
   private:
     std::vector<std::shared_ptr<GenericFunction>> ComponentVec_;
};

class SensorProcessing {
  public:
    void Configure (const rapidjson::Value& Config);
    bool Initialized ();
    void RunBaseline (GenericFunction::Mode mode);
    void SetTest (std::string TestSel);
    std::string GetTest ();
    void RunTest (GenericFunction::Mode mode);
  private:
    std::string RootPath_ = "/Sensor-Processing";
    std::string BaselinePath_, TestPath_;

    bool InitializedLatch_ = false;

    typedef std::shared_ptr<SystemWrapper> SystemWrapperPtr ;

    SystemWrapperPtr BaselinePtr_;

    std::vector<std::string> SystemDefKeys_;
    std::map<std::string, SystemWrapperPtr> SystemMap_;

    std::string TestSel_;

    std::string FmuGroup_;
    std::string BaselineGroup_;
    std::vector<std::string> TestGroups_;

    std::map<std::string, std::vector<std::string>> TestKeys;

    typedef std::vector<ElementPtr> NodeVec;
    typedef std::map<std::string, NodeVec> NodeMap;
    typedef std::map<std::string, NodeMap> NodeSetMap;

    NodeSetMap BaselineNodeMap_, TestNodeMap_;
};
