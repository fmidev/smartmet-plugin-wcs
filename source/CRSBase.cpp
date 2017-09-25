#include "CRSBase.h"

namespace SmartMet
{
namespace Plugin
{
namespace WCS
{
void CRSBase::setAbbrev(const Abbrev& abbrev)
{
  mAbbrev = abbrev;
}

void CRSBase::setIdentifier(const Identifier& identifier)
{
  mIdentifier = identifier;
}

void CRSBase::parse(boost::shared_ptr<SmartMet::Spine::ConfigBase> config,
                    libconfig::Setting& setting)
{
  config->assert_is_group(setting);

  CRSBase::Identifier identifier =
      config->get_mandatory_config_param<CRSBase::Identifier>(setting, "identifier");

  CRSBase::Abbrev abbrev = config->get_mandatory_config_param<CRSBase::Abbrev>(setting, "abbrev");
  if (abbrev.empty())
  {
    std::ostringstream msg;
    msg << "Abbreviation (abbrev) of '" << identifier << "' CRS configuration cannot be empty.";
    throw std::runtime_error(msg.str());
  }

  CRSBase::setIdentifier(identifier);
  CRSBase::setAbbrev(abbrev);
}
}
}
}
