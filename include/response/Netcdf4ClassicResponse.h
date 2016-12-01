#pragma once

#include "NetcdfResponse.h"

namespace SmartMet
{
namespace Plugin
{
namespace WCS
{
class Netcdf4ClassicResponse : public NetcdfResponse
{
 public:
  Netcdf4ClassicResponse() {}
  ~Netcdf4ClassicResponse() {}
  void get(std::ostream& output);
};
}
}
}
