#if (!defined(__CINT__) && !defined(__CLING__)) || defined(__MAKECINT__)
  #include <iostream>

  #include "Rtypes.h"
  #include "TString.h"
  #include "TStopwatch.h"
  #include "TGeoManager.h"

  #include "FairLogger.h"
  #include "FairRunAna.h"
  #include "FairFileSource.h"
  #include "FairSystemInfo.h"
  #include "FairRuntimeDb.h"
  #include "FairParRootFileIo.h"
  #include "FairLinkManager.h"

  #include "TGeoGlobalMagField.h"
  #include "Field/MagneticField.h"

  #include "TPCSimulation/DigitizerTask.h"
#endif

void run_digi_tpc(Int_t nEvents = 10, TString mcEngine = "TGeant3", Int_t isContinuous=1){
        // Initialize logger
        FairLogger *logger = FairLogger::GetLogger();
        logger->SetLogVerbosityLevel("LOW");
        logger->SetLogScreenLevel("DEBUG");

        // Input and output file name
        std::stringstream inputfile, outputfile, paramfile;
        inputfile << "AliceO2_" << mcEngine << ".tpc.mc_" << nEvents << "_event.root";
        paramfile << "AliceO2_" << mcEngine << ".tpc.params_" << nEvents << ".root";
        outputfile << "AliceO2_" << mcEngine << ".tpc.digi_" << nEvents << "_event.root";

        // Setup timer
        TStopwatch timer;

        // Setup FairRoot analysis manager
        FairRunAna * run = new FairRunAna();

        // ===| Activation of fair links for MC ID |============================
        run->SetUseFairLinks(kTRUE);
        // -- only store the link to the MC track
        //    if commented, also the links to all previous steps will be stored
        FairLinkManager::Instance()->AddIncludeType(0);

        FairFileSource *fFileSource = new FairFileSource(inputfile.str().c_str());
        run->SetSource(fFileSource);
        run->SetOutputFile(outputfile.str().c_str());

        // Setup Runtime DB
        FairRuntimeDb* rtdb = run->GetRuntimeDb();
        FairParRootFileIo* parInput1 = new FairParRootFileIo();
        parInput1->open(paramfile.str().c_str());
        rtdb->setFirstInput(parInput1);

        fFileSource->SetEventMeanTime(20*1000); //is in us
      //  TGeoManager::Import("geofile_full.root");

        o2::field::MagneticField *magField = new o2::field::MagneticField("Maps","Maps", -1., -1., o2::field::MagFieldParam::k5kG);
        run->SetField(magField);

        // Setup digitizer
        o2::TPC::DigitizerTask *digiTPC = new o2::TPC::DigitizerTask;
        digiTPC->setContinuousReadout(isContinuous);

        run->AddTask(digiTPC);

        run->Init();

        timer.Start();
        run->Run();

        std::cout << std::endl << std::endl;

        // Extract the maximal used memory an add is as Dart measurement
        // This line is filtered by CTest and the value send to CDash
        FairSystemInfo sysInfo;
        Float_t maxMemory=sysInfo.GetMaxMemory();
        std::cout << R"(<DartMeasurement name="MaxMemory" type="numeric/double">)";
        std::cout << maxMemory;
        std::cout << "</DartMeasurement>" << std::endl;

        timer.Stop();
        Double_t rtime = timer.RealTime();
        Double_t ctime = timer.CpuTime();

        Float_t cpuUsage=ctime/rtime;
        std::cout << R"(<DartMeasurement name="CpuLoad" type="numeric/double">)";
        std::cout << cpuUsage;
        std::cout << "</DartMeasurement>" << std::endl;
        std::cout << std::endl << std::endl;
        std::cout << "Macro finished succesfully." << std::endl;
        std::cout << std::endl << std::endl;
        std::cout << "Output file is "    << outputfile.str() << std::endl;
        //std::cout << "Parameter file is " << parFile << std::endl;
        std::cout << "Real time " << rtime << " s, CPU time " << ctime
                  << "s" << std::endl << std::endl;


}
