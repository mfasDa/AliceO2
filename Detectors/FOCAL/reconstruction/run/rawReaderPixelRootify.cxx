// Copyright 2019-2020 CERN and copyright holders of ALICE O2.
// See https://alice-o2.web.cern.ch/copyright for details of the copyright holders.
// All rights not expressly granted are reserved.
//
// This software is distributed under the terms of the GNU General Public
// License v3 (GPL Version 3), copied verbatim in the file "COPYING".
//
// In applying this license CERN does not waive the privileges and immunities
// granted to it by virtue of its status as an Intergovernmental Organization
// or submit itself to any jurisdiction.

/// \file rawReaderFileNew.cxx
/// \author Markus Fasel <markus.fasel@cern.ch>, Oak Ridge National Laboratory

#include <bitset>
#include <cstdlib>
#include <iostream>
#include <map>
#include <set>
#include <vector>
#include <boost/program_options.hpp>
#include <gsl/span>
#include <fairlogger/Logger.h>

#include <TFile.h>
#include <TTree.h>

#include "CommonConstants/Triggers.h"
#include "DetectorsRaw/HBFUtils.h"
#include "DetectorsRaw/RawFileReader.h"
#include "DetectorsRaw/RDHUtils.h"
#include "FOCALReconstruction/PixelDecoder.h"
#include "Headers/RDHAny.h"

namespace bpo = boost::program_options;

class HBFHandler
{
 public:
  HBFHandler() = default;
  ~HBFHandler() = default;

  void clear() { mHBFs.clear(); };

  void insert(int link, const o2::InteractionRecord& ir)
  {
    auto linkFound = mHBFs.find(link);
    if (linkFound != mHBFs.end()) {
      linkFound->second.insert(ir);
    } else {
      std::set<o2::InteractionRecord> irs;
      irs.insert(ir);
      mHBFs[link] = irs;
    }
  }

  int getNumberOfHBFs() const
  {
    return getMaxNumberOfHBFs();
  }

  int getNumberOfLinks() const { return mHBFs.size(); }

  int getNumberOfHBFsForLink(int link) const
  {
    auto found = mHBFs.find(link);
    if (found != mHBFs.end()) {
      return found->second.size();
    }
    return 0;
  }

  int getMaxNumberOfHBFs() const
  {
    int max = 0;
    for (const auto& [link, hbfs] : mHBFs) {
      if (hbfs.size() > max) {
        max = hbfs.size();
      }
    }
    return max;
  }

  bool linksConsistent() const
  {
    std::set<o2::InteractionRecord> cache;
    bool first = true, mismatch = false;
    for (const auto& currentLink : mHBFs) {
      if (first) {
        cache = currentLink.second;
        first = false;
      } else {
        if (currentLink.second != cache) {
          mismatch = true;
        }
      }
    }
    return !mismatch;
  }

  const std::set<o2::InteractionRecord> getInteractionRecordsForLink(int link) const
  {
    auto found = mHBFs.find(link);
    if (found != mHBFs.end()) {
      return found->second;
    }
    return std::set<o2::InteractionRecord>();
  }

  std::set<int> getLinks() const
  {
    std::set<int> result;
    for (auto& [link, hbfs] : mHBFs) {
      result.insert(link);
    }
    return result;
  }

 private:
  std::map<int, std::set<o2::InteractionRecord>> mHBFs;
};

int main(int argc, char** argv)
{
  bpo::variables_map vm;
  bpo::options_description opt_general("Usage:\n  " + std::string(argv[0]) +
                                       " <cmds/options>\n"
                                       "  Tool will decode the DDLx data for EMCAL 0\n"
                                       "Commands / Options");
  bpo::options_description opt_hidden("");
  bpo::options_description opt_all;
  bpo::positional_options_description opt_pos;

  try {
    auto add_option = opt_general.add_options();
    add_option("help,h", "Print this help message");
    add_option("verbose,v", bpo::value<uint32_t>()->default_value(0), "Select verbosity level [0 = no output]");
    add_option("version", "Print version information");
    add_option("input-file,i", bpo::value<std::string>()->required(), "Specifies input file. Multiple files can be parsed separated by ,");
    add_option("output-file,o", bpo::value<std::string>()->default_value("FOCALPadData.root"), "Output file for rootified data");
    add_option("readout,r", bpo::value<std::string>()->default_value("RORC"), "Readout mode (RORC or CRU)");
    add_option("firstorbit", bpo::value<uint32_t>()->default_value(0), "First orbit (default: 0 -> autodetect)");
    add_option("autodect_tf0", bpo::value<bool>()->default_value(false), "Auto-detect first orbit");
    add_option("debug,d", bpo::value<uint32_t>()->default_value(0), "Select debug output level [0 = no debug output]");

    opt_all.add(opt_general).add(opt_hidden);
    bpo::store(bpo::command_line_parser(argc, argv).options(opt_all).positional(opt_pos).run(), vm);

    if (vm.count("help") || argc == 1) {
      std::cout << opt_general << std::endl;
      exit(0);
    }

    if (vm.count("version")) {
      // std::cout << GitInfo();
      exit(0);
    }

    bpo::notify(vm);
  } catch (bpo::error& e) {
    std::cerr << "ERROR: " << e.what() << std::endl
              << std::endl;
    std::cerr << opt_general << std::endl;
    exit(1);
  } catch (std::exception& e) {
    std::cerr << e.what() << ", application will now exit" << std::endl;
    exit(2);
  }

  auto rawfilename = vm["input-file"].as<std::string>();
  auto rootfilename = vm["output-file"].as<std::string>();
  auto readoutmode = vm["readout"].as<std::string>();

  std::vector<std::string> inputfiles;
  if (rawfilename.find(",") != std::string::npos) {
    std::stringstream parser(rawfilename);
    std::string buffer;
    while (std::getline(parser, buffer, ',')) {
      LOG(info) << "Adding " << buffer;
      inputfiles.push_back(buffer);
    }
    LOG(info) << "Found " << inputfiles.size() << " input files to process";
  }

  auto firstorbit = vm["firstorbit"].as<uint32_t>();
  if (firstorbit > 0) {
    o2::raw::HBFUtils::setValue("HBFUtils", "orbitFirst", firstorbit);
  }

  o2::raw::RawFileReader::ReadoutCardType readout = o2::raw::RawFileReader::RORC;
  if (readoutmode != "RORC" && readoutmode != "CRU") {
    LOG(error) << "Unknown readout mode - select RORC or CRU";
    exit(3);
  } else if (readoutmode == "RORC") {
    LOG(info) << "Reconstructing in C-RORC mode";
    readout = o2::raw::RawFileReader::RORC;
  } else {
    LOG(info) << "Reconstructing in CRU mode";
    readout = o2::raw::RawFileReader::CRU;
  }

  o2::raw::RawFileReader reader;
  reader.setDefaultDataOrigin(o2::header::gDataOriginITS);
  reader.setDefaultDataDescription(o2::header::gDataDescriptionRawData);
  reader.setDefaultReadoutCardType(readout);
  if (vm["autodect_tf0"].as<bool>()) {
    reader.setTFAutodetect(o2::raw::RawFileReader::FirstTFDetection::Pending);
  }
  reader.setPreferCalculatedTFStart(true);
  for (auto rawfile : inputfiles) {
    reader.addFile(rawfile);
  }
  reader.init();

  int nHBFprocessed = 0, nTFprocessed = 0, nEventsProcessed = 0;
  std::map<int, int> nEvnetsHBF;
  while (1) {
    int tfID = reader.getNextTFToRead();
    std::cout << "Processing TF " << tfID << std::endl;
    if (tfID >= reader.getNTimeFrames()) {
      LOG(info) << "nothing left to read after " << tfID << " TFs read";
      break;
    }
    std::vector<char> rawtf; // where to put extracted data
    int nlinks = 0;
    HBFHandler handler;
    for (int il = 0; il < reader.getNLinks(); il++) {
      auto& link = reader.getLink(il);

      auto sz = link.getNextTFSize(); // size in bytes needed for the next TF of this link
      if (sz) {
        nlinks++;
      }
      rawtf.resize(sz);
      link.readNextTF(rawtf.data());

      int currentpos = 0;
      o2::InteractionRecord currentir;
      bool newHBD = true;
      std::vector<char> hbfbuffer;
      while (currentpos < rawtf.size()) {
        auto rdh = reinterpret_cast<const o2::header::RDHAny*>(rawtf.data() + currentpos);
        auto trigger = o2::raw::RDHUtils::getTriggerType(rdh);
        auto feeID = o2::raw::RDHUtils::getFEEID(rdh);
        if (trigger & o2::trigger::ORBIT && trigger & o2::trigger::HB) {
          if (newHBD) {
            // new HBF
            newHBD = false;
            auto orbit = o2::raw::RDHUtils::getTriggerOrbit(rdh);
            auto bc = o2::raw::RDHUtils::getTriggerBC(rdh);
            currentir.bc = bc;
            currentir.orbit = orbit;
            handler.insert(feeID, currentir);
          } else if (o2::raw::RDHUtils::getStop(rdh)) {
            // close HBF
            newHBD = true;
          }
        }
        currentpos += o2::raw::RDHUtils::getOffsetToNext(rdh);
      }
    }
    std::cout << "Received data for " << nlinks << " links" << std::endl;
    if (handler.linksConsistent()) {
      auto nHBF = handler.getMaxNumberOfHBFs();
      std::cout << "Received " << nHBF << " for each link" << std::endl;
      nHBFprocessed += nHBF;
    } else {
      std::cout << "Inconsistent amount of HBF for links: " << std::endl;
      for (auto link : handler.getLinks()) {
        std::cout << "Link 0x" << std::hex << link << std::dec << " received " << handler.getNumberOfHBFsForLink(link) << std::endl;
        bool first = true;
        for (auto& ir : handler.getInteractionRecordsForLink(link)) {
          if (first) {
            first = false;
          } else {
            std::cout << ", ";
          }
          std::cout << "(" << ir.orbit << " / " << ir.bc << ")";
        }
        std::cout << std::endl;
      }
      nHBFprocessed += handler.getMaxNumberOfHBFs();
    }
    reader.setNextTFToRead(++tfID);
    nTFprocessed++;
  }
  LOG(info) << "Processed " << nTFprocessed << " timeframes, " << nHBFprocessed << " HBFs";
  LOG(info) << "Analyzed " << nEventsProcessed << " events:";
  LOG(info) << "=============================================================";
  for (auto& [nevents, nHBF] : nEvnetsHBF) {
    LOG(info) << "  " << nevents << " event(s)/HBF: " << nHBF << " HBFs ...";
  }
  return EXIT_SUCCESS;
}