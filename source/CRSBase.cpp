#include "CRSBase.h"

#include <boost/algorithm/string.hpp>
#include <list>

namespace SmartMet
{
namespace Plugin
{
namespace WCS
{
CRSBase::CRSBase() : mDimensionSize(0)
{
}

void CRSBase::setAbbrev(const Abbrev& abbrev)
{
  if (abbrev.empty())
  {
    std::ostringstream msg;
    msg << "Abbreviation of an CRS can not be an empty string.";
    throw std::runtime_error(msg.str());
  }

  std::list<Abbrev> abbrevList;
  boost::algorithm::split(abbrevList, abbrev, boost::algorithm::is_any_of(" "));
  mDimensionSize = abbrevList.size();

  if (mDimensionSize > 3 or mDimensionSize < 1)
  {
    std::ostringstream msg;
    msg << "Abbreviation '" << abbrev << "' contains more than 3 parts.";
    throw std::runtime_error(msg.str());
  }

  mDimensionSize = abbrevList.size();
  mAbbrev = abbrev;
}

void CRSBase::setIdentifier(const Identifier& identifier)
{
  if (identifier.empty())
  {
    std::ostringstream msg;
    msg << "Identifier of an CRS can not be an empty string.";
    throw std::runtime_error(msg.str());
  }

  mIdentifier = identifier;
}

void CRSBase::parse(boost::shared_ptr<SmartMet::Spine::ConfigBase> config,
                    libconfig::Setting& setting)
{
  config->assert_is_group(setting);

  CRSBase::Identifier identifier =
      config->get_mandatory_config_param<CRSBase::Identifier>(setting, "identifier");

  CRSBase::Abbrev abbrev = config->get_mandatory_config_param<CRSBase::Abbrev>(setting, "abbrev");
  CRSBase::setIdentifier(identifier);
  CRSBase::setAbbrev(abbrev);
}
}
}
}
