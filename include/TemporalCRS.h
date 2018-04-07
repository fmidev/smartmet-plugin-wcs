#pragma once

#include "CRSBase.h"

#include <boost/optional.hpp>
#include <boost/shared_ptr.hpp>
#include <set>

namespace SmartMet
{
namespace Plugin
{
namespace WCS
{
/**
@verbatim
{
  identifier="http://www.opengis.net/def/crs/OGC/0/UnixTime";
  abbrev = "unix";
};
@endverbatim
*/

class TemporalCRS : public CRSBase
{
 public:
  using Optional = boost::optional<TemporalCRS>;
  TemporalCRS(boost::shared_ptr<SmartMet::Spine::ConfigBase> config, libconfig::Setting& setting);
  virtual ~TemporalCRS() {}

 private:
  using ValidIdentifiers = std::set<CRSBase::Identifier>;
  ValidIdentifiers mValidIdentifiers = {
      "http://www.opengis.net/def/crs/OGC/0/AnsiDate",
      "http://www.opengis.net/def/crs/OGC/0/ChronometricGeologicTime",
      "http://www.opengis.net/def/crs/OGC/0/JulianDate",
      "http://www.opengis.net/def/crs/OGC/0/UnixTime"};
};
}  // namespace WCS
}  // namespace Plugin
}  // namespace SmartMet
