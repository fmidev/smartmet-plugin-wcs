#pragma once

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
  crs = "http://www.opengis.net/def/crs/EPSG/0/4258";

  // Optional VerticalCRS configuration
  vertical_crs: { };

  // Optional TemporalCRS configuration
  temporal_crs: { };
};
@endverbatim
 */
class CompoundCRS
{
 public:
  using Optional = boost::optional<CompoundCRS>;
  using CRSType = std::string;

  CompoundCRS(boost::shared_ptr<SmartMet::Spine::ConfigBase> config, libconfig::Setting& setting);
  virtual ~CompoundCRS();

  inline const CRSType& getCrs() const { return mCrs; }
  inline const VerticalCRS::Optional& getVerticalCRS() const { return mVerticalCRS; }
  inline const TemporalCRS::Optional& getTemporalCRS() const { return mTemporalCRS; }

 private:
  std::string mCrs;
  VerticalCRS::Optional mVerticalCRS;
  TemporalCRS::Optional mTemporalCRS;
};
}
}
}
