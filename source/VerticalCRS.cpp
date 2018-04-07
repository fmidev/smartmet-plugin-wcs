#include "VerticalCRS.h"

namespace SmartMet
{
namespace Plugin
{
namespace WCS
{
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
}  // namespace WCS
}  // namespace Plugin
}  // namespace SmartMet
