#pragma once

#include "PluginData.h"
#include "RequestFactory.h"
#include <spine/HTTP.h>
#include <spine/Reactor.h>
#include <spine/SmartMetPlugin.h>

#include <ctpp2/CDT.hpp>

#include <macgyver/TemplateFormatterMT.h>

#include <iostream>
#include <list>
#include <map>
#include <string>

namespace SmartMet
{
namespace Plugin
{
namespace WCS
{
class Plugin : public SmartMetPlugin, virtual private boost::noncopyable, private Xml::EnvInit
{
  class RequestResult;

 public:
  Plugin(SmartMet::Spine::Reactor* reactor, const char* config);
  virtual ~Plugin();

  const std::string& getPluginName() const;
  int getRequiredAPIVersion() const;

 private:
  Plugin();

  void query(const SmartMet::Spine::HTTP::Request& req, RequestResult& result);

  bool queryIsFast(const SmartMet::Spine::HTTP::Request& request) const;

 private:
  void realRequestHandler(SmartMet::Spine::Reactor& reactor,
                          const SmartMet::Spine::HTTP::Request& request,
                          SmartMet::Spine::HTTP::Response& response);

  void maybeValidateOutput(const SmartMet::Spine::HTTP::Request& req,
                           SmartMet::Spine::HTTP::Response& response) const;

  boost::optional<std::string> getFmiApikey(const SmartMet::Spine::HTTP::Request& request) const;

  Xml::Parser* getXmlParser() const;

  RequestBaseP parseKvpGetCapabilitiesRequest(const SmartMet::Spine::HTTP::Request& request);

  RequestBaseP parseXmlGetCapabilitiesRequest(const xercesc::DOMDocument& document,
                                              const xercesc::DOMElement& root);

  RequestBaseP parseKvpDescribeCoverageRequest(const SmartMet::Spine::HTTP::Request& request);

  RequestBaseP parseXmlDescribeCoverageRequest(const xercesc::DOMDocument& document,
                                               const xercesc::DOMElement& root);

  RequestBaseP parseKvpGetCoverageRequest(const SmartMet::Spine::HTTP::Request& request);

  RequestBaseP parseXmlGetCoverageRequest(const xercesc::DOMDocument& document,
                                          const xercesc::DOMElement& root);

 protected:
  void init();

  void shutdown() {}
  void requestHandler(SmartMet::Spine::Reactor& reactor,
                      const SmartMet::Spine::HTTP::Request& request,
                      SmartMet::Spine::HTTP::Response& response);

 private:
  const std::string mModuleName;

  std::unique_ptr<PluginData> mPluginData;
  std::unique_ptr<QueryResponseCache> mQueryCache;

  /**
   *   @brief An object that reads actual requests and creates request objects
   */
  std::unique_ptr<RequestFactory> mRequestFactory;

  SmartMet::Spine::Reactor* mReactor;

  const char* mConfig;
};
}
}
}
