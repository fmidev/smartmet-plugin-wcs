#include "CompoundCRS.h"

#include <boost/algorithm/string.hpp>
#include <macgyver/StringConversion.h>

namespace SmartMet
{
namespace Plugin
{
namespace WCS
{
CompoundCRS::CompoundCRS(boost::shared_ptr<SmartMet::Spine::ConfigBase> config,
                         libconfig::Setting& setting)
{
  config->assert_is_group(setting);

  libconfig::Setting* crs = config->find_setting(setting, "crs", true);
  CRSBase::parse(config, *crs);

  libconfig::Setting* verticalCRS = config->find_setting(setting, "vertical_crs", false);
  if (verticalCRS)
    mVerticalCRS = VerticalCRS(config, *verticalCRS);

  libconfig::Setting* temporalCRS = config->find_setting(setting, "temporal_crs", false);
  if (temporalCRS)
    mTemporalCRS = TemporalCRS(config, *temporalCRS);
}

CompoundCRS::~CompoundCRS()
{
}

Abbreviations::Shared CompoundCRS::getAbbreviations() const
{
  Abbreviations::Vector abbrVector;
  boost::algorithm::split(abbrVector, getAbbrev(), boost::algorithm::is_any_of(" "));

  if (mVerticalCRS)
    abbrVector.push_back(mVerticalCRS->getAbbrev());
  else
    abbrVector.push_back("");

  if (mTemporalCRS)
    abbrVector.push_back(mTemporalCRS->getAbbrev());
  else
    abbrVector.push_back("");

  Abbreviations::Shared result(new Abbreviations(abbrVector));
  return result;
}
}
}
}
