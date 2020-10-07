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


const double ENERGY_TRUNCATION = 0.0153;

Cell::Cell()
{
  memset(mCellWords, 0, sizeof(uint16_t) * 3);
}

Cell::Cell(Short_t tower, Double_t energy, Double_t time, ChannelType_t ctype)
{
  memset(mCellWords, 0, sizeof(uint16_t) * 3);
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
  getDataRepresentation()->mTime = timestamp;
}

Short_t Cell::getTimeStamp() const
{
  return getDataRepresentation()->mTime;
}

void Cell::setEnergy(Double_t energy)
{
  double truncatedEnergy = energy;
  if (truncatedEnergy < 0.) {
    truncatedEnergy = 0.;
  } else if (truncatedEnergy > 250.) {
    truncatedEnergy = 250.;
  }
  auto convertedEnergy = static_cast<int16_t>(truncatedEnergy / ENERGY_TRUNCATION);
  getDataRepresentation()->mEnergy = convertedEnergy;
}

Double_t Cell::getEnergy() const
{
  auto convertedEnergy = static_cast<double>(getDataRepresentation()->mEnergy) * ENERGY_TRUNCATION;
  return convertedEnergy;
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
