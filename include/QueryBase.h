#pragma once

#include <boost/optional/optional.hpp>
#include <ostream>
#include <string>

namespace SmartMet
{
namespace Plugin
{
namespace WCS
{
class QueryBase
{
 public:
  static const char* FMI_APIKEY_SUBST;
  static const char* FMI_APIKEY_PREFIX_SUBST;
  static const char* HOSTNAME_SUBST;

 public:
  QueryBase();
  virtual ~QueryBase();
};
}
}
}
