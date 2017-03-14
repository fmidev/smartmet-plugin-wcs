#include "CompoundCRS.h"
#include <macgyver/StringConversion.h>

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

  CRSBase::setIdentifier(identifier);
  CRSBase::setAbbrev(abbrev);
}

TemporalCRS::TemporalCRS(boost::shared_ptr<SmartMet::Spine::ConfigBase> config,
                         libconfig::Setting& setting)
    : CRSBase()
{
  config->assert_is_group(setting);
  CRSBase::parse(config, setting);
  if (mValidIdentifiers.find(CRSBase::getIdentifier()) == mValidIdentifiers.end())
  {
    std::ostringstream msg;
    msg << "ERROR: Unsupported temporal CRS identifier value '" << CRSBase::getIdentifier()
        << "' in '" << config->get_file_name() << "' dataset configurantion.\n"
        << "Supported values are:";
    for (auto& item : mValidIdentifiers)
      msg << " '" << item << "'";
    throw std::runtime_error(msg.str());
  }
}

VerticalCRS::VerticalCRS(boost::shared_ptr<SmartMet::Spine::ConfigBase> config,
                         libconfig::Setting& setting)
    : CRSBase()
{
  config->assert_is_group(setting);
  CRSBase::parse(config, setting);
  if (mValidIdentifiers.find(CRSBase::getIdentifier()) == mValidIdentifiers.end())
  {
    std::ostringstream msg;
    msg << "ERROR: Unsupported vertical CRS identifier value '" << CRSBase::getIdentifier()
        << "' in '" << config->get_file_name() << "' dataset configurantion.\n"
        << "Supported values are:";
    for (auto& item : mValidIdentifiers)
      msg << " '" << item << "'";
    throw std::runtime_error(msg.str());
  }
}

CompoundCRS::CompoundCRS(boost::shared_ptr<SmartMet::Spine::ConfigBase> config,
                         libconfig::Setting& setting)
{
  config->assert_is_group(setting);
  mCrs = config->get_mandatory_config_param<std::string>(setting, "crs");

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
}
}
}
