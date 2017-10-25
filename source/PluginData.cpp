#include "PluginData.h"
#include "WcsException.h"

#include <macgyver/StringConversion.h>
#include <macgyver/TimeParser.h>
#include <functional>

namespace SmartMet
{
namespace Plugin
{
namespace WCS
{
namespace pt = boost::posix_time;

PluginData::PluginData(Spine::Reactor *reactor, const char *config)
    : mConfig(config), mWcsCapabilities(new WcsCapabilities)
{
  if (config == NULL)
  {
    std::ostringstream msg;
    msg << "ERROR: No configuration provided for WCS plugin";
    throw std::runtime_error(msg.str());
  }

  Spine::Reactor::EngineInstance engine = reactor->getSingleton("Querydata", NULL);
  if (engine == NULL)
    throw std::runtime_error("No Querydata Engine available");
  mQuerydataEngine = reinterpret_cast<Engine::Querydata::Engine *>(engine);

  engine = reactor->getSingleton("Gis", NULL);
  if (engine == NULL)
    throw std::runtime_error("No GisEngine available");
  mGisEngine = reinterpret_cast<Engine::Gis::Engine *>(engine);

  createTemplateFormatters();
  createXmlParser();
  createParameterConfigs();
  createServiceMetaData();

  std::vector<boost::shared_ptr<DataSetDef> > dsDefs = mConfig.readDataSetDefs();
  for (auto &dsDef : dsDefs)
  {
    if (dsDef->getDisabled())
      continue;
    if (not dsDef->process(*mQuerydataEngine))
      continue;

    for (auto &param : dsDef->getParameters())
    {
      CoverageDescription cov;
      cov.setProducer(dsDef->getName());
      cov.setParameterName(param);
      cov.setDataSetDef(dsDef);
      std::string covName = dsDef->getName();
      covName.append("-").append(param);
      mWcsCapabilities->registerDataset(Fmi::ascii_tolower_copy(covName), cov);
    }
  }

  std::string sLockedTimestamp;
  if (mConfig.get_config().lookupValue("lockedTimeStamp", sLockedTimestamp))
  {
    mLockedTimeStamp = Fmi::TimeParser::parse(sLockedTimestamp);
    std::cout << "*****************************************************************\n";
    std::cout << "Using fixed timestamp " << (Fmi::to_iso_extended_string(*mLockedTimeStamp) + "Z")
              << std::endl;
    std::cout << "*****************************************************************\n";
  }
}

PluginData::~PluginData()
{
}
boost::posix_time::ptime PluginData::getTimeStamp() const
{
  if (mLockedTimeStamp)
  {
    return *mLockedTimeStamp;
  }
  else
  {
    return pt::second_clock::universal_time();
  }
}

boost::posix_time::ptime PluginData::getLocalTimeStamp() const
{
  if (mLockedTimeStamp)
  {
    return *mLockedTimeStamp;
  }
  else
  {
    return pt::second_clock::universal_time();
  }
}

void PluginData::createTemplateFormatters()
{
  const std::string gctn =
      mConfig.get_mandatory_config_param<std::string>("getCapabilitiesTemplate");
  const std::string dcovtn =
      mConfig.get_mandatory_config_param<std::string>("describeCoverageTemplate");
  const std::string gcovtn = mConfig.get_mandatory_config_param<std::string>("getCoverageTemplate");
  const std::string etn = mConfig.get_mandatory_config_param<std::string>("exceptionTemplate");
  const std::string dfn = mConfig.get_mandatory_config_param<std::string>("ctppDumpTemplate");

  mGetCapabilitiesFormatter = mConfig.getTemplateDirectory().create_template_formatter(gctn);
  mDescribeCoverageFormatter = mConfig.getTemplateDirectory().create_template_formatter(dcovtn);
  mGetCoverageFormatter = mConfig.getTemplateDirectory().create_template_formatter(gcovtn);
  mExceptionFormatter = mConfig.getTemplateDirectory().create_template_formatter(etn);
  mCtppDumpFormatter = mConfig.getTemplateDirectory().create_template_formatter(dfn);
}

void PluginData::createXmlParser()
{
  const std::string xmlGrammarPoolFn = mConfig.getXMLGrammarPoolDumpFn();
  std::cout << "\t\t+ [Using XML grammar pool dump file '" << xmlGrammarPoolFn << "']" << std::endl;
  mXmlParser.reset(new Xml::ParserMT(xmlGrammarPoolFn, false));

  std::string serializedXmlSchemas =
      mConfig.get_optional_config_param<std::string>("serializedXmlSchemas", "");
  if (serializedXmlSchemas != "")
  {
    mXmlParser->load_schema_cache(serializedXmlSchemas);
  }
}

void PluginData::createParameterConfigs()
{
  const ParamConfig::FilePathType &netcdfParamConfigPath =
      mConfig.get_mandatory_config_param<std::string>("netcdfParamConfigPath");
  mNetcdfParamConfig.reset(new NetcdfParamConfig(netcdfParamConfigPath));
}

void PluginData::createServiceMetaData()
{
  try
  {
    libconfig::Setting *setting = mConfig.find_setting(mConfig.get_root(), "Capabilities", false);
    if (setting)
    {
      WcsCapabilities::ServiceMetaDataMap metaDataMap;
      const auto &defaultLanguage = getLanguages().at(0);

      for (auto language : getLanguages())
      {
        WcsCapabilities::ServiceMetaData serviceMetaData;
        serviceMetaData.setDefaultLanguage(defaultLanguage);
        serviceMetaData.setLanguage(language);
        serviceMetaData.set(*setting);
        mWcsCapabilities->registerServiceMetaData(language, serviceMetaData);
      }

      std::vector<WcsCapabilities::ServiceMetaData::Path> settingsRequired = {
          "Capabilities.ServiceIdentification.Title",
          "Capabilities.ServiceIdentification.Abstract",
          "Capabilities.ServiceIdentification.Keywords",
          "Capabilities.ServiceIdentification.Keywords.Keyword",
          "Capabilities.ServiceIdentification.ServiceType",
          "Capabilities.ServiceIdentification.Fees",
          "Capabilities.ServiceIdentification.AccessConstraints",
          "Capabilities.ServiceProvider.ProviderName",
          "Capabilities.ServiceProvider.ProviderSite",
          "Capabilities.ServiceProvider.ProviderSite.href",
          "Capabilities.ServiceProvider.ServiceContact",
          "Capabilities.ServiceProvider.ServiceContact.ContactInfo",
          "Capabilities.ServiceProvider.ServiceContact.ContactInfo.Phone",
          "Capabilities.ServiceProvider.ServiceContact.ContactInfo.Phone.Voice",
          "Capabilities.ServiceProvider.ServiceContact.ContactInfo.Phone.Facsimile",
          "Capabilities.ServiceProvider.ServiceContact.ContactInfo.Address",
          "Capabilities.ServiceProvider.ServiceContact.ContactInfo.Address.DeliveryPoint",
          "Capabilities.ServiceProvider.ServiceContact.ContactInfo.Address.City",
          "Capabilities.ServiceProvider.ServiceContact.ContactInfo.Address.AdministrativeArea",
          "Capabilities.ServiceProvider.ServiceContact.ContactInfo.Address.PostalCode",
          "Capabilities.ServiceProvider.ServiceContact.ContactInfo.Address.Country",
          "Capabilities.ServiceProvider.ServiceContact.ContactInfo.Address.ElectronicMailAddress",
          "Capabilities.ServiceProvider.ServiceContact.ContactInfo.OnlineResource",
          "Capabilities.ServiceProvider.ServiceContact.ContactInfo.OnlineResource.href",
          "Capabilities.ServiceProvider.ServiceContact.ContactInfo.ContactInstructions",
          "Capabilities.ServiceProvider.ServiceContact.Role"};

      for (const auto &path : settingsRequired)
      {
        if (not mWcsCapabilities->getServiceMetaData(defaultLanguage)->exists(path))
        {
          std::ostringstream msg;
          msg << "Configuration '" << path
              << "' not found from the main configurations. Following setting required: \n";
          for (const auto &item : settingsRequired)
            msg << item << "\n";

          throw WcsException(WcsException::NO_APPLICABLE_CODE, msg.str());
        }
      }
    }
  }
  catch (const std::exception &exception)
  {
    throw WcsException(WcsException::NO_APPLICABLE_CODE, exception.what());
  }
  catch (...)
  {
    throw WcsException(WcsException::NO_APPLICABLE_CODE,
                       "Failed to read or set ServiceMetaData setting from main configuration.");
  }
}
}
}
}
