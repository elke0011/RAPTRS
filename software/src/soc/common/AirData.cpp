/*
Copyright (c) 2016 - 2019 Regents of the University of Minnesota and Bolder Flight Systems Inc.
MIT License; See LICENSE.md for complete details
Author: Brian Taylor and Chris Regan
*/

#include "AirData.h"

/* computes indicated airspeed given differential pressure */
float AirData::getIAS(float qc) {
  float temp = 5.0f*(powf((qc/P0 + 1.0f),(2.0f/7.0f)) - 1.0f);
  if (temp > 0) {
    return A0 * sqrtf(temp);
  } else {
    return A0 * -sqrtf(-temp);
  }
}

/* computes equivalent airspeed given differential pressure and static pressure */
float AirData::getEAS(float qc, float p) {
  float temp = 5.0f*p/P0*(powf((qc/p + 1.0f),(2.0f/7.0f)) - 1.0f);
  if (temp > 0) {
    return A0 * sqrtf(temp);
  } else {
    return A0 * -sqrtf(-temp);
  }
}

/* computes true air speed given IAS or EAS and temperature */
float AirData::getTAS(float AS, float T) {
  if (T > -273.15) {
    return AS * sqrtf((T+273.15f)/T0);
  } else {
    return 0.0;
  }
}

/* computes pressure altitude given static pressure */
float AirData::getPressureAltitude(float p) {
  return (T0/L)*(1.0f - powf((p/P0),((L*R)/(M*g))));
}

/* computes altitude Above Ground Level (AGL) given static pressure and a bias */
float AirData::getAGL(float p, float c) {
  return getPressureAltitude(p) - c;
}

/* computes altitude above Mean Sea Level (MSL) given AGL and starting altitude */
float AirData::getMSL(float H, float h) {
  return H + h;
}

/* computes density altitude given static pressure and temperature */
float AirData::getDensityAltitude(float p, float T) {
  return (T0/L)*(1.0f - powf(((p/P0)*(T0/(T+273.15f))),((L*R)/(M*g - L*R))));
}

/* approximates temperature as a fuction of altitude */
float AirData::getApproxTemp(float T, float h) {
  return T - L*h;
}

/* computes air density given temperature and pressure */
float AirData::getDensity(float p, float T) {
  return (M*p)/(R*(T+273.15f));
}

// Angle of Attack or Sidslip estimate when all ports are measuring pressure difference from static ring
// For alpha: Angle ports are alpha, Side ports are beta
// For beta: Angle ports are beta, Side ports are alpha
float AirData::getAngle1(float pTip, float pAngle1, float pAngle2, float pSide1, float pSide2, float kCal) {

  float qcn = pTip - 0.5 * (pSide1 + pSide2);
  float angle;

  if (qcn != 0) { // Dived by zero protection
    angle = (pAngle1 - pAngle2) / (kCal * qcn);
  } else {
    angle = 0.0;
  }

  return angle;
}

// Angle of Attack estimate when pAlpha is measuring pAlpha1-pAlpha2 directly
float AirData::getAngle2(float pTip, float pAngle, float kCal) {

  float angle = pAngle / (kCal * pTip);

  return angle;
}
