// Copyright CERN and copyright holders of ALICE O2. This software is
// distributed under the terms of the GNU General Public License v3 (GPL
// Version 3), copied verbatim in the file "COPYING".
//
// See https://alice-o2.web.cern.ch/ for full licensing information.
//
// In applying this license CERN does not waive the privileges and immunities
// granted to it by virtue of its status as an Intergovernmental Organization
// or submit itself to any jurisdiction.

#pragma once

#include <TTree.h>

#include "QCProducer/Producer.h"

namespace o2
{
namespace qc
{
class TreeProducer : public Producer
{
 public:
  TreeProducer(const char* treeName, const char* treeTitle, const int numberOfBranches,
               const int numberOfEntriesInEachBranch);
  TObject* produceData() const override;

 private:
  const char* mTreeName;
  const char* mTreeTitle;
  const int mNumberOfBranches;
  const int mNumberOfEntriesInEachBranch;

  void createBranch(TTree* tree, int brunchNumber, const char* branchNamePrefix = "default_branch_name_") const;
};
}
}