#include "DataSetDef.h"
#include <macgyver/StringConversion.h>

namespace SmartMet
{
namespace Plugin
{
namespace WCS
{
DataSetDef::DataSetDef(const std::string& defaultLanguage,
                       boost::shared_ptr<SmartMet::Spine::ConfigBase> config,
                       libconfig::Setting& setting)
{
  mDisabled = config->get_optional_config_param<bool>(setting, "disabled", false);
  if (mDisabled)
    return;

  mName = config->get_mandatory_config_param<std::string>(setting, "name");

  if (libconfig::Setting* mlTitle = config->find_setting(setting, "title", false))
    mTitle = MultiLanguageString::create(defaultLanguage, *mlTitle);

  if (libconfig::Setting* mlAbstract = config->find_setting(setting, "abstract", false))
    mAbstract = MultiLanguageString::create(defaultLanguage, *mlAbstract);

  if (config->find_setting(setting, "keywords", false))
  {
    Keywords tmpKeywords;
    config->get_config_array(setting, "keywords", tmpKeywords, 1);
    mKeywords = tmpKeywords;
  }

  config->get_config_array(setting, "parameters", mParameters);
  mSubtype = config->get_optional_config_param<std::string>(setting, "subtype", "GridCoverage");

  libconfig::Setting* datasetSetting = config->find_setting(setting, "dataset", false);
  if (datasetSetting)
    mDataset = Dataset(config, *datasetSetting);

  libconfig::Setting* compoundcrsSetting = config->find_setting(setting, "compoundcrs", false);
  if (compoundcrsSetting)
    mCompoundcrs = CompoundCRS(config, *compoundcrsSetting);

  if (not mCompoundcrs)
  {
    std::ostringstream msg;
    msg << "Compoundcrs must be defined in '" << mName << "' dataset configuration\n";
    throw std::runtime_error(msg.str());
  }
}

DataSetDef::~DataSetDef()
{
}

boost::optional<DataSetDef::Message> DataSetDef::getAbstract(const Language& language) const
{
  return (mAbstract ? mAbstract->get(language) : boost::optional<Message>());
}

boost::optional<DataSetDef::Message> DataSetDef::getTitle(const Language& language) const
{
  return (mTitle ? mTitle->get(language) : boost::optional<Message>());
}
bool DataSetDef::process(const Engine::Querydata::Engine& engine)
{
  std::list<Engine::Querydata::MetaData> metaDataList = engine.getEngineMetadata();
  boost::optional<Engine::Querydata::MetaData> metaData;
  for (const auto& md : metaDataList)
  {
    if (md.producer == mName)
    {
      metaData = md;
      break;
    }
  }

  if (!metaData)
  {
    std::ostringstream msg;
    msg << "WARNING: No metadata provided for '" << mName << "' data set.\n";
    std::cerr << msg.str();
    return false;
  }

  const std::set<std::string> levelsSupported = {"AnyLevelType", "Depth", "PressureLevel"};
  for (auto& level : metaData->levels)
  {
    if (levelsSupported.find(level.type) == levelsSupported.end())
    {
      std::ostringstream msg;
      msg << "ERROR: Unsupported level type '" << level.type << "' in the data of '" << mName
          << "' dataset configurantion.\n";
      throw std::runtime_error(msg.str());
    }
  }

  mWGS84BBox = metaData->wgs84Envelope;

  return true;
}
}
}
}
