#pragma once

#include "Abbreviations.h"
#include "TemporalCRS.h"
#include "VerticalCRS.h"

namespace SmartMet
{
namespace Plugin
{
namespace WCS
{
/**
@verbatim
{
  // Mandatory crs configuration
  crs:
  {
    identifier = "http://www.opengis.net/def/crs/EPSG/0/4258";
    abbrev = "Lat Long";
  };

  // Optional VerticalCRS configuration
  vertical_crs: { };

  // Optional TemporalCRS configuration
  temporal_crs: { };
};
@endverbatim
 */
class CompoundCRS : public CRSBase
{
 public:
  using Optional = boost::optional<CompoundCRS>;

  CompoundCRS(boost::shared_ptr<SmartMet::Spine::ConfigBase> config, libconfig::Setting& setting);
  virtual ~CompoundCRS();

  inline const VerticalCRS::Optional& getVerticalCRS() const { return mVerticalCRS; }
  inline const TemporalCRS::Optional& getTemporalCRS() const { return mTemporalCRS; }

  Abbreviations::Shared getAbbreviations() const;

 private:
  VerticalCRS::Optional mVerticalCRS;
  TemporalCRS::Optional mTemporalCRS;
};
}
}
}
