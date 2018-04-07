#pragma once

#include "NetcdfResponse.h"

namespace SmartMet
{
namespace Plugin
{
namespace WCS
{
class Netcdf4ClassicSampleResponse : public NetcdfResponse
{
 public:
  Netcdf4ClassicSampleResponse() {}
  ~Netcdf4ClassicSampleResponse() {}
  void get(std::ostream& output);
};
}  // namespace WCS
}  // namespace Plugin
}  // namespace SmartMet
