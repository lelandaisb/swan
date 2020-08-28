target_sources(swangeo PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/SwanGEOFunctions.cc)
#include_directories(/ccc/products/netcdf-c-4.6.0/intel--17.0.4.196/hdf5__serial/include/)
#target_link_directories(swangeo PUBLIC /ccc/products/netcdf-c-4.6.0/intel--17.0.4.196/hdf5__serial/lib/)
target_link_libraries(swangeo PUBLIC netcdf)
