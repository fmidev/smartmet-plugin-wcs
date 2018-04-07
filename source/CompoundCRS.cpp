#include "CompoundCRS.h"
#include "WcsException.h"

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

  Abbreviations::Vector abbrVector;
  boost::algorithm::split(abbrVector, getAbbrev(), boost::algorithm::is_any_of(" "));

  if (abbrVector.size() < 2 or abbrVector.size() > 3)
  {
    std::ostringstream msg;
    msg << "2 or 3 abbreviations required for '" << getIdentifier() << "'.";
    throw WcsException(WcsException::NO_APPLICABLE_CODE, msg.str());
  }

  if (mVerticalCRS and abbrVector.size() > 2)
  {
    std::ostringstream msg;
    msg << "Three abbreviations for '" << getIdentifier() << "' "
        << "and vertical CRS '" << mVerticalCRS->getIdentifier()
        << "' detected in same compoundcrs configuration. "
        << "Maybe there is too many abbreviations defined.";
    throw WcsException(WcsException::NO_APPLICABLE_CODE, msg.str());
  }
}

CompoundCRS::~CompoundCRS() {}

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
}  // namespace WCS
}  // namespace Plugin
}  // namespace SmartMet
