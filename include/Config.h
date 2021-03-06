#pragma once

#include "DataSetDef.h"
#include <boost/optional.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/utility.hpp>
#include <spine/CRSRegistry.h>
#include <spine/ConfigBase.h>
#include <libconfig.h++>
#include <string>
#include <vector>

namespace SmartMet
{
namespace Plugin
{
namespace WCS
{
/**
 *   @brief The configuration of WCS plugin
 */
class Config : private boost::noncopyable, public SmartMet::Spine::ConfigBase
{
 public:
  Config(const std::string& configfile);

  const int& getDebugLevel() const { return mDebugLevel; }
  inline const std::string& defaultUrl() const { return mDefaultUrl; }
  inline const std::string& getFallbackHostname() const { return mFallbackHostname; }
  const std::string getXMLGrammarPoolDumpFn() const { return mXmlGrammarPoolDump; }
  bool getValidateXmlOutput() const { return mValidateOutput; }
  int getDefaultExpiresSeconds() const { return mDefaultExpiresSeconds; }
  const boost::filesystem::path& getTemplateDirectory() const { return *mTemplateDirectory; }
  const std::vector<std::string>& getLanguages() const { return mLanguages; }
  const std::vector<std::string>& getCswLanguages() const { return mCswLanguages; }
  const std::vector<std::string>& getSupportedCrss() const { return mSupportedCrss; }
  const std::string& getDefaultCrs() const { return mSupportedCrss.at(0); }
  const std::string& getCompoundcrsUri() const { return mCompoundcrsUri; }
  inline int getCacheSize() const { return mCacheSize; }
  inline int getCacheTimeConstant() const { return mCacheTimeConstant; }
  std::vector<boost::shared_ptr<DataSetDef> > readDataSetDefs();

 private:
  int mDebugLevel;
  std::string mDefaultUrl;
  std::string mFallbackHostname;
  int mCacheSize;
  int mCacheTimeConstant;
  int mDefaultExpiresSeconds;
  std::vector<std::string> mLanguages;
  std::vector<std::string> mSupportedCrss;
  std::vector<std::string> mCswLanguages;
  std::string mCompoundcrsUri;
  boost::optional<boost::filesystem::path> mTemplateDirectory;
  std::string mXmlGrammarPoolDump;
  bool mValidateOutput;
};
}  // namespace WCS
}  // namespace Plugin
}  // namespace SmartMet
