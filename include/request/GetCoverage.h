#pragma once

#include "Options.h"
#include "PluginData.h"
#include "QueryBase.h"
#include "RequestBase.h"
#include <macgyver/Cache.h>
#include <xercesc/dom/DOMDocument.hpp>

namespace SmartMet
{
namespace Plugin
{
namespace WCS
{
namespace Request
{
/**
 *   @brief Represents GetCoverage request
 */
class GetCoverage : public RequestBase
{
 public:
  GetCoverage(PluginData& pluginData);

  virtual ~GetCoverage();

  virtual RequestType getType() const;

  virtual void execute(std::ostream& ost) const;

  virtual int getResponseExpiresSeconds() const;

  static boost::shared_ptr<GetCoverage> createFromKvp(
      const SmartMet::Spine::HTTP::Request& httpRequest,
      PluginData& pluginData,
      QueryResponseCache& queryCache);

  static boost::shared_ptr<GetCoverage> createFromXml(const xercesc::DOMDocument& document,
                                                      PluginData& pluginData,
                                                      QueryResponseCache& queryCache);

  void setQueryCache(QueryResponseCache& queryCache) { mQueryCache = queryCache; }
  const SupportedFormatsType& getSupportedFormats() const { return mSupportedFormats; }

 protected:
  void checkXmlAttributes(const xercesc::DOMDocument& document);
  void checkKvpAttributes(const SmartMet::Spine::HTTP::Request& request);
  void checkCoverageIds(const std::vector<std::string>& ids);
  void setCoverageIds(const CoverageIdsType& ids);
  void setOutputFormat(const RequestBase::FormatType& format);
  std::shared_ptr<Options> getOptions() { return mOptions; }
  std::shared_ptr<Options> mOptions;

 private:
  bool getCachedResponses();

  void executeSingleQuery(std::ostream& ost) const;

  void setProducer();
  void setParameterName();
  void setAbbreviations();
  void setTransformation();

  boost::shared_ptr<DataSetDef> getDataSetDef() const;

 private:
  PluginData& mPluginData;
  boost::optional<QueryResponseCache&> mQueryCache;
  RequestBase::SupportedFormatsType mSupportedFormats;
  RequestBase::CoverageIdsType mCoverageIds;
  boost::optional<RequestBase::FormatType> mOutputFormat;
  bool mFast;
};
}  // namespace Request
}  // namespace WCS
}  // namespace Plugin
}  // namespace SmartMet
