target_sources(swan PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/SwanFunctions.cc)
#include_directories(/ccc/products/netcdf-c-4.6.0/intel--17.0.4.196/hdf5__serial/include/)
#target_link_directories(swan PUBLIC /ccc/products/netcdf-c-4.6.0/intel--17.0.4.196/hdf5__serial/lib/)
target_link_libraries(swan PUBLIC netcdf)
