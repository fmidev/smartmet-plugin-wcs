#pragma once

#include <boost/shared_ptr.hpp>
#include <engines/querydata/Engine.h>
#include <macgyver/TemplateFormatterMT.h>
#include "RequestBase.h"
#include "PluginData.h"

namespace SmartMet
{
namespace Plugin
{
namespace WCS
{
namespace Request
{
/**
 *   @brief Class for representing DescribeCoverage WCS request
 */
class DescribeCoverage : public RequestBase
{
  const char* DATA_CRS_NAME = "urn:ogc:def:crs:EPSG::4326";

 public:
  DescribeCoverage(const PluginData& pluginData);

  virtual ~DescribeCoverage();

  virtual RequestType getType() const;

  virtual void execute(std::ostream& ost) const;

  static boost::shared_ptr<DescribeCoverage> createFromKvp(
      const SmartMet::Spine::HTTP::Request& http_request, const PluginData& pluginData);

  static boost::shared_ptr<DescribeCoverage> createFromXml(const xercesc::DOMDocument& document,
                                                           const PluginData& pluginData);

  virtual int getResponseExpiresSeconds() const;

  const SupportedFormatsType& getSupportedFormats() const { return mSupportedFormats; }
 protected:
  void checkKvpAttributes(const SmartMet::Spine::HTTP::Request& request);
  void checkKvpAcceptLanguages(const SmartMet::Spine::HTTP::Request& httpRequest);
  void checkXmlAttributes(const xercesc::DOMDocument& document);
  void checkXmlAcceptLanguages(const xercesc::DOMDocument& document);
  void checkCoverageIds();

  /**
   *   @brief Validate languages against the values allowed.
   */
  void checkLanguages(const std::vector<std::string>& valuesAllowed,
                      const std::vector<std::string>& languages);

  void setCoverageIds(const std::vector<std::string>& ids);

  NFmiPoint transform(const std::unique_ptr<OGRCoordinateTransformation>& transformation,
                      const NFmiPoint&& p) const;
  std::string epochToString(const boost::posix_time::ptime&& epoch,
                            const std::string&& encodingStyle = "unixtime") const;

 private:
  const PluginData& mPluginData;
  std::vector<std::string> mAcceptLanguages;
  std::vector<std::string> mIds;
  boost::optional<std::string> mRequestedLanguage;
  RequestBase::SupportedFormatsType mSupportedFormats;
};
}
}
}
}
