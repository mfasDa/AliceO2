set(MODULE_NAME "ITSMFTSimulation")

O2_SETUP(NAME ${MODULE_NAME})

set(SRCS
    src/Point.cxx
    src/ClusterShape.cxx
    )
set(HEADERS
    include/${MODULE_NAME}/Point.h
    include/${MODULE_NAME}/ClusterShape.h
    )
set(NO_DICT_SRCS # sources not for the dictionary
    src/Chip.cxx
    src/DigitChip.cxx
    src/DigitContainer.cxx
    src/SimuClusterShaper.cxx
    src/SimulationAlpide.cxx
   )
set(NO_DICT_HEADERS # sources not for the dictionary
    include/${MODULE_NAME}/Chip.h
    include/${MODULE_NAME}/DigitChip.h
    include/${MODULE_NAME}/DigitContainer.h
    include/${MODULE_NAME}/SimuClusterShaper.h
    include/${MODULE_NAME}/SimulationAlpide.h
   )
Set(LINKDEF src/ITSMFTSimulationLinkDef.h)
Set(LIBRARY_NAME ${MODULE_NAME})
set(BUCKET_NAME itsmft_simulation_bucket)

O2_GENERATE_LIBRARY()

