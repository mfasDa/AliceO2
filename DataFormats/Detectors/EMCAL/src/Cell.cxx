// Copyright CERN and copyright holders of ALICE O2. This software is
// distributed under the terms of the GNU General Public License v3 (GPL
// Version 3), copied verbatim in the file "COPYING".
//
// See http://alice-o2.web.cern.ch/license for full licensing information.
//
// In applying this license CERN does not waive the privileges and immunities
// granted to it by virtue of its status as an Intergovernmental Organization
// or submit itself to any jurisdiction.

#include "DataFormatsEMCAL/Constants.h"
#include "DataFormatsEMCAL/Cell.h"
#include <iostream>
#include <bitset>

using namespace o2::emcal;

Cell::Cell()
{
  auto tmp = reinterpret_cast<uint16_t*>(&mCellWord);
  tmp[0] = tmp[1] = tmp[2] = 0;
}

Cell::Cell(Short_t tower, Double_t energy, Double_t time, ChannelType_t ctype)
{
  setTower(tower);
  setTimeStamp(time);
  setEnergy(energy);
  setType(ctype);
}

void Cell::setTimeStamp(Short_t timestamp)
{
  const int TIME_MIN = -1023,
            TIME_MAX = 1023;
  // truncate:
  if (timestamp < TIME_MIN)
    timestamp = TIME_MIN;
  else if (timestamp > TIME_MAX)
    timestamp = TIME_MAX;
  mCellWord.mTime = timestamp;
}

Short_t Cell::getTimeStamp() const
{
  return mCellWord.mTime;
}

void Cell::setEnergy(Double_t energy)
{
  double truncatedEnergy = energy;
  if (truncatedEnergy < 0.) {
    truncatedEnergy = 0.;
  } else if (truncatedEnergy > 250.) {
    truncatedEnergy = 250.;
  }
  mCellWord.mEnergy = static_cast<uint16_t>(truncatedEnergy / 0.0153);
}

Double_t Cell::getEnergy() const
{
  return static_cast<double>(mCellWord.mEnergy) * 0.0153;
}

void Cell::PrintStream(std::ostream& stream) const
{
  stream << "EMCAL Cell: Type " << getType() << ", Energy " << getEnergy() << ", Time " << getTimeStamp() << ", Tower " << getTower();
}

std::ostream& operator<<(std::ostream& stream, const Cell& c)
{
  c.PrintStream(stream);
  return stream;
}
