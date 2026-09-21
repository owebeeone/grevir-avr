include(CMakeFindDependencyMacro)
find_dependency(grevir-registers CONFIG)
find_dependency(grevir-core CONFIG)
include("${CMAKE_CURRENT_LIST_DIR}/GrevirAVRTargets.cmake")
