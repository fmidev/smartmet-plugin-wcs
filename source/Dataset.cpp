#include "Dataset.h"

namespace SmartMet
{
namespace Plugin
{
namespace WCS
{
Dataset::Dataset(boost::shared_ptr<SmartMet::Spine::ConfigBase> config, libconfig::Setting& setting)

{
  config->assert_is_group(setting);
  mId = config->get_mandatory_config_param<std::string>(setting, "id");

  auto ns = config->get_optional_config_param<std::string>(setting, "ns", "");
  if (not ns.empty())
    mIdNs = ns;

  auto uuid = config->get_optional_config_param<std::string>(setting, "uuid", "");
  if (not uuid.empty())
    mUuid = uuid;
}
}
}
}
