#pragma once

#include "Config.h"
#include "ParamConfig.h"
#include "WcsCapabilities.h"
#include "xml/XmlEnvInit.h"
#include "xml/XmlParser.h"
#include <boost/date_time/posix_time/ptime.hpp>
#include <boost/shared_ptr.hpp>
#include <engines/gis/Engine.h>
#include <engines/querydata/Engine.h>
#include <macgyver/Cache.h>
#include <macgyver/TemplateFactory.h>
#include <spine/CRSRegistry.h>
#include <spine/Reactor.h>
#include <memory>

namespace SmartMet
{
namespace Plugin
{
namespace WCS
{
using QueryResponseCache = Fmi::Cache::Cache<std::string,
                                             std::string,
                                             Fmi::Cache::LRUEviction,
                                             std::string,
                                             Fmi::Cache::InstantExpire>;

class PluginData : public boost::noncopyable
{
 public:
  PluginData(SmartMet::Spine::Reactor* reactor, const char* config);
  virtual ~PluginData();

  inline Spine::CRSRegistry& getCrsRegistry() const { return mGisEngine->getCRSRegistry(); }
  inline Config& getConfig() { return mConfig; }
  inline const Config& getConfig() const { return mConfig; }

  inline boost::shared_ptr<Fmi::TemplateFormatter> getGetCapabilitiesFormatter() const
  {
    return mTemplateFactory.get(mGetCapabilitiesFormatterPath);
  }

  inline boost::shared_ptr<Fmi::TemplateFormatter> getDescribeCoverageFormatter() const
  {
    return mTemplateFactory.get(mDescribeCoverageFormatterPath);
  }

  inline boost::shared_ptr<Fmi::TemplateFormatter> getGetCoverageFormatter() const
  {
    return mTemplateFactory.get(mGetCoverageFormatterPath);
  }

  inline boost::shared_ptr<Fmi::TemplateFormatter> getExceptionFormatter() const
  {
    return mTemplateFactory.get(mExceptionFormatterPath);
  }

  inline boost::shared_ptr<Fmi::TemplateFormatter> getCtppDumpFormatter() const
  {
    return mTemplateFactory.get(mCtppDumpFormatterPath);
  }

  inline std::shared_ptr<NetcdfParamConfig> getNetcdfParamConfig() const
  {
    return mNetcdfParamConfig;
  }

  inline boost::shared_ptr<Xml::ParserMT> getXmlParser() const { return mXmlParser; }
  inline const std::vector<std::string>& getLanguages() const { return mConfig.getLanguages(); }
  inline const std::vector<std::string>& getCswLanguages() const
  {
    return mConfig.getCswLanguages();
  }
  boost::posix_time::ptime getTimeStamp() const;
  boost::posix_time::ptime getLocalTimeStamp() const;
  inline WcsCapabilities& getCapabilities() { return *mWcsCapabilities; }
  inline const WcsCapabilities& getCapabilities() const { return *mWcsCapabilities; }
  inline const Engine::Querydata::Engine* getQuerydataEngine() const { return mQuerydataEngine; }
  inline const Engine::Gis::Engine* getGisEngine() const { return mGisEngine; }

 private:
  void createTemplateFormatterPaths();
  void createXmlParser();
  void createParameterConfigs();
  void createServiceMetaData();

 private:
  Config mConfig;

  SmartMet::Engine::Querydata::Engine* mQuerydataEngine;
  SmartMet::Engine::Gis::Engine* mGisEngine;

  Fmi::TemplateFactory mTemplateFactory;
  boost::filesystem::path mGetCapabilitiesFormatterPath;
  boost::filesystem::path mDescribeCoverageFormatterPath;
  boost::filesystem::path mGetCoverageFormatterPath;
  boost::filesystem::path mExceptionFormatterPath;
  boost::filesystem::path mCtppDumpFormatterPath;
  boost::shared_ptr<Xml::ParserMT> mXmlParser;
  std::unique_ptr<WcsCapabilities> mWcsCapabilities;
  std::shared_ptr<NetcdfParamConfig> mNetcdfParamConfig;

  /**
   *   @brief Locked timestamp for testing only
   */
  boost::optional<boost::posix_time::ptime> mLockedTimeStamp;
};
}  // namespace WCS
}  // namespace Plugin
}  // namespace SmartMet
