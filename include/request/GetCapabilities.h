#pragma once

#include "PluginData.h"
#include "RequestBase.h"
#include <boost/shared_ptr.hpp>

namespace SmartMet
{
namespace Plugin
{
namespace WCS
{
namespace Request
{
/**
 *   @brief Class for representing GetCapabilities WCS request
 */
class GetCapabilities : public RequestBase
{
 public:
  GetCapabilities(const PluginData& plugin_data);

  virtual ~GetCapabilities();

  virtual RequestType getType() const;

  virtual void execute(std::ostream& ost) const;

  static boost::shared_ptr<GetCapabilities> createFromKvp(
      const SmartMet::Spine::HTTP::Request& httpRequest, const PluginData& pluginData);

  static boost::shared_ptr<GetCapabilities> createFromXml(const xercesc::DOMDocument& document,
                                                          const PluginData& pluginData);

  virtual int getResponseExpiresSeconds() const;

  void checkXmlAttributes(const xercesc::DOMDocument& document);
  void checkKvpAttributes(const SmartMet::Spine::HTTP::Request& httpRequest);

  const SupportedFormatsType& getSupportedFormats() const { return mSupportedFormats; }

 private:
  void checkVersions(const std::vector<std::string>& versions);

  /**
   *   @brief Validate languages against the values allowed.
   */
  void checkLanguages(const std::vector<std::string>& valuesAllowed,
                      const std::vector<std::string>& languages);

  const PluginData& mPluginData;
  std::vector<std::string> mAcceptLanguages;
  std::vector<std::string> mAcceptVersions;
  boost::optional<std::string> mResponseVersion;
  RequestBase::SupportedFormatsType mSupportedFormats;
};
}  // namespace Request
}  // namespace WCS
}  // namespace Plugin
}  // namespace SmartMet
