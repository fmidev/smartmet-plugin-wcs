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
  identifier="http://www.opengis.net/def/crs/EPSG/0/5715";
  abbrev="Depth";
};
@endverbatim
*/

class VerticalCRS : public CRSBase
{
 public:
  using Optional = boost::optional<VerticalCRS>;
  VerticalCRS(boost::shared_ptr<SmartMet::Spine::ConfigBase> config, libconfig::Setting& setting);
  virtual ~VerticalCRS() {}

 private:
  using ValidIdentifiers = std::set<CRSBase::Identifier>;
  ValidIdentifiers mValidIdentifiers = {"http://www.opengis.net/def/crs/EPSG/0/5730",
                                        "http://www.opengis.net/def/crs/EPSG/0/5861",
                                        "http://www.opengis.net/def/crs/EPSG/0/5715",
                                        "pressure"};
};
}
}
}
