set(MODULE_NAME "DetectorsPassive")

O2_SETUP(NAME ${MODULE_NAME})

set(SRCS
  src/Cave.cxx
  src/GeoCave.cxx
  src/Pipe.cxx
  src/Magnet.cxx
  src/PassiveContFact.cxx
  src/FrameStructure.cxx
)

Set(HEADERS
    include/${MODULE_NAME}/Cave.h
    include/${MODULE_NAME}/GeoCave.h
    include/${MODULE_NAME}/Magnet.h
    include/${MODULE_NAME}/PassiveContFact.h
    include/${MODULE_NAME}/Pipe.h
    include/${MODULE_NAME}/FrameStructure.h
 )

Set(LINKDEF src/PassiveLinkDef.h)
Set(LIBRARY_NAME ${MODULE_NAME})
set(BUCKET_NAME fairroot_geom)

O2_GENERATE_LIBRARY()
