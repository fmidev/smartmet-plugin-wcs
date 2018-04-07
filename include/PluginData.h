#pragma once

#include <engines/gis/CRSRegistry.h>
#include <engines/gis/Engine.h>
#include <engines/querydata/Engine.h>
#include <spine/Reactor.h>

#include "Config.h"
#include "ParamConfig.h"
#include "WcsCapabilities.h"
#include "xml/XmlEnvInit.h"
#include "xml/XmlParser.h"
#include <boost/date_time/posix_time/ptime.hpp>
#include <boost/shared_ptr.hpp>
#include <macgyver/Cache.h>
#include <macgyver/TemplateFormatterMT.h>
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

  inline Engine::Gis::CRSRegistry& getCrsRegistry() const { return mGisEngine->getCRSRegistry(); }
  inline Config& getConfig() { return mConfig; }
  inline const Config& getConfig() const { return mConfig; }
  inline boost::shared_ptr<Fmi::TemplateFormatterMT> getGetCapabilitiesFormater() const
  {
    return mGetCapabilitiesFormatter;
  }

  inline boost::shared_ptr<Fmi::TemplateFormatterMT> getDescribeCoverageFormater() const
  {
    return mDescribeCoverageFormatter;
  }

  inline boost::shared_ptr<Fmi::TemplateFormatterMT> getGetCoverageFormater() const
  {
    return mGetCoverageFormatter;
  }

  inline boost::shared_ptr<Fmi::TemplateFormatterMT> getExceptionFormatter() const
  {
    return mExceptionFormatter;
  }

  inline boost::shared_ptr<Fmi::TemplateFormatterMT> getCtppDumpFormatter() const
  {
    return mCtppDumpFormatter;
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
  void createTemplateFormatters();
  void createXmlParser();
  void createParameterConfigs();
  void createServiceMetaData();

 private:
  Config mConfig;

  SmartMet::Engine::Querydata::Engine* mQuerydataEngine;
  SmartMet::Engine::Gis::Engine* mGisEngine;

  boost::shared_ptr<Fmi::TemplateFormatterMT> mGetCapabilitiesFormatter;
  boost::shared_ptr<Fmi::TemplateFormatterMT> mDescribeCoverageFormatter;
  boost::shared_ptr<Fmi::TemplateFormatterMT> mGetCoverageFormatter;
  boost::shared_ptr<Fmi::TemplateFormatterMT> mExceptionFormatter;
  boost::shared_ptr<Fmi::TemplateFormatterMT> mCtppDumpFormatter;
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
