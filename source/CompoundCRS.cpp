#include "CompoundCRS.h"
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
}
}
}
