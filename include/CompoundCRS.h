#pragma once

#include <boost/optional.hpp>
#include <boost/shared_ptr.hpp>
#include <spine/ConfigBase.h>
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
  crs = "http://www.opengis.net/def/crs/EPSG/0/4258";
  vertical_crs:
  {
    identifier="http://www.opengis.net/def/crs/EPSG/0/5715";
    abbrev="Depth";
  };
  temporal_crs:
  {
    identifier="http://www.opengis.net/def/crs/OGC/0/UnixTime";
    abbrev = "unix";
  };
};
@endverbatim
 */

class CRSBase
{
 public:
  using Identifier = std::string;
  using Abbrev = std::string;
  CRSBase() {}
  virtual ~CRSBase() {}
  inline const Abbrev& getAbbrev() const { return mAbbrev; }
  inline const Identifier& getIdentifier() const { return mIdentifier; }

 protected:
  void parse(boost::shared_ptr<SmartMet::Spine::ConfigBase> config, libconfig::Setting& setting);
  void setAbbrev(const Abbrev& abbrev);
  void setIdentifier(const Identifier& identifier);

 private:
  Identifier mIdentifier;
  Abbrev mAbbrev;
};

class TemporalCRS : public CRSBase
{
 public:
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

class VerticalCRS : public CRSBase
{
 public:
  VerticalCRS(boost::shared_ptr<SmartMet::Spine::ConfigBase> config, libconfig::Setting& setting);
  virtual ~VerticalCRS() {}

 private:
  using ValidIdentifiers = std::set<CRSBase::Identifier>;
  ValidIdentifiers mValidIdentifiers = {"http://www.opengis.net/def/crs/EPSG/0/5730",
                                        "http://www.opengis.net/def/crs/EPSG/0/5861",
                                        "http://www.opengis.net/def/crs/EPSG/0/5715",
                                        "pressure"};
};

class CompoundCRS
{
 public:
  using CRSType = std::string;

  CompoundCRS(boost::shared_ptr<SmartMet::Spine::ConfigBase> config, libconfig::Setting& setting);
  virtual ~CompoundCRS();

  inline const CRSType& getCrs() const { return mCrs; }
  inline const boost::optional<VerticalCRS>& getVerticalCRS() const { return mVerticalCRS; }
  inline const boost::optional<TemporalCRS>& getTemporalCRS() const { return mTemporalCRS; }

 private:
  std::string mCrs;
  boost::optional<VerticalCRS> mVerticalCRS;
  boost::optional<TemporalCRS> mTemporalCRS;
};
}
}
}
