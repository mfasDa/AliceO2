// Copyright CERN and copyright holders of ALICE O2. This software is
// distributed under the terms of the GNU General Public License v3 (GPL
// Version 3), copied verbatim in the file "COPYING".
//
// See https://alice-o2.web.cern.ch/ for full licensing information.
//
// In applying this license CERN does not waive the privileges and immunities
// granted to it by virtue of its status as an Intergovernmental Organization
// or submit itself to any jurisdiction.

/// \file ElectronTransport.cxx
/// \brief Implementation of the electron transport
/// \author Andi Mathis, TU München, andreas.mathis@ph.tum.de

#include "TPCSimulation/ElectronTransport.h"
#include "TPCSimulation/Constants.h"

#include <cmath>

using namespace o2::TPC;

ElectronTransport::ElectronTransport()
  : mRandomGaus()
  , mRandomFlat()
{
  mRandomGaus.initialize(RandomRing::RandomType::Gaus);
  mRandomFlat.initialize(RandomRing::RandomType::Flat);
}

ElectronTransport::~ElectronTransport()
= default;

GlobalPosition3D ElectronTransport::getElectronDrift(GlobalPosition3D posEle)
{
  /// For drift lengths shorter than 1 mm, the drift length is set to that value
  float driftl = posEle.getZ();
  if(driftl<0.01) {
    driftl=0.01;
  }
  driftl = std::sqrt(driftl);
  const float sigT = driftl*DIFFT;
  const float sigL = driftl*DIFFL;
  
  /// The position is smeared by a Gaussian with mean around the actual position and a width according to the diffusion coefficient times sqrt(drift length)
  GlobalPosition3D posEleDiffusion((mRandomGaus.getNextValue() * sigT) + posEle.getX(),
                                   (mRandomGaus.getNextValue() * sigT) + posEle.getY(),
                                   (mRandomGaus.getNextValue() * sigL) + posEle.getZ());
  return posEleDiffusion;
}
