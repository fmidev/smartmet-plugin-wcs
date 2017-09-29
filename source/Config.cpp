#include "Config.h"
#include <boost/algorithm/string.hpp>
#include <boost/filesystem.hpp>
#include <boost/format.hpp>
#include <spine/Convenience.h>
#include <sstream>
#include <stdexcept>

using namespace std;

namespace fs = boost::filesystem;
namespace ba = boost::algorithm;

namespace SmartMet
{
namespace Plugin
{
namespace WCS
{
const char* default_url = "/wcs";

Config::Config(const std::string& configfile)
    : ConfigBase(configfile, "WCS plugin"), mDefaultUrl(default_url)
{
  mDebugLevel = get_optional_config_param<int>("debugLevel", 1);
  mDefaultUrl = get_optional_config_param<std::string>("url", mDefaultUrl);
  mFallbackHostname = get_optional_config_param<std::string>("fallback_hostname", "localhost");
  mCacheSize = get_optional_config_param<int>("cacheSize", 100);
  mCacheTimeConstant = get_optional_config_param<int>("cacheTimeConstant", 60);
  mDefaultExpiresSeconds = get_optional_config_param<int>("defaultExpiresSeconds", 60);
  const std::string templateDir = get_mandatory_config_param<std::string>("templateDir");
  mLanguages = get_mandatory_config_array<std::string>("languages", 1);
  for (std::string& language : mLanguages)
  {
    Fmi::ascii_tolower(language);
  }

  // The first one is used as the default.
  auto supportedCRSs = get_mandatory_config_array<std::string>("supportedCRSs", 1);
  for (std::string& crs : supportedCRSs)
  {
    auto tmp = ba::trim_copy(crs);
    if (tmp.empty())
      throw std::runtime_error("Empty supportedCRSs string");
    mSupportedCrss.push_back(tmp);
  }

  mCswLanguages = get_mandatory_config_array<std::string>("cswLanguages", 1);
  for (auto& language : mCswLanguages)
    Fmi::ascii_tolower(language);

  mCswUri = get_mandatory_config_param<std::string>("cswURI");

  mCompoundcrsUri = get_optional_config_param<std::string>(
      "compoundcrsURI", "http://www.opengis.net/def/crs-compound");

  std::vector<std::string> xmlGrammarPoolFns;
  // Ensure that setting exists (or report an error otherwise)
  get_mandatory_config_param<libconfig::Setting&>("xmlGrammarPoolDump");
  // Not get the array.
  bool xgpFound = false;
  get_config_array("xmlGrammarPoolDump", xmlGrammarPoolFns, 1);
  for (const auto& fn : xmlGrammarPoolFns)
  {
    if (fs::exists(fn))
    {
      xgpFound = true;
      mXmlGrammarPoolDump = fn;
      break;
    }
  }

  if (not xgpFound)
  {
    std::ostringstream msg;
    msg << METHOD_NAME << ": no usable XML grammar pool file found";
    throw std::runtime_error(msg.str());
  }

  if (mXmlGrammarPoolDump == "" or not fs::exists(mXmlGrammarPoolDump))
  {
    std::ostringstream msg;
    msg << METHOD_NAME << ": XML grammar pool file '" << mXmlGrammarPoolDump << "' not found";
    throw std::runtime_error(msg.str());
  }

  // Verify that template directory exists
  fs::path sqtd(templateDir);
  mTemplateDirectory.reset(new Fmi::TemplateDirectory(templateDir));
}

std::vector<boost::shared_ptr<DataSetDef> > Config::readDataSetDefs()
{
  fs::path dataSetDir(get_mandatory_config_param<std::string>("dataSetDir"));

  if (not fs::exists(dataSetDir))
  {
    std::ostringstream msg;
    msg << METHOD_NAME << ": '" << dataSetDir << "' not found";
    throw std::runtime_error(msg.str());
  }

  if (not fs::is_directory(dataSetDir))
  {
    std::ostringstream msg;
    msg << METHOD_NAME << ": '" << dataSetDir << "' is not a directory";
    throw std::runtime_error(msg.str());
  }

  std::vector<boost::shared_ptr<DataSetDef> > result;

  for (auto it = fs::directory_iterator(dataSetDir); it != fs::directory_iterator(); ++it)
  {
    const fs::path entry = *it;
    const auto fn = entry.filename().string();
    if (fs::is_regular_file(entry) and not ba::starts_with(fn, ".") and
        not ba::starts_with(fn, "#") and ba::ends_with(fn, ".conf"))
    {
      boost::shared_ptr<SmartMet::Spine::ConfigBase> desc(
          new ConfigBase(entry.string(), "Data set definition"));

      try
      {
        boost::shared_ptr<DataSetDef> dataSetDef(
            new DataSetDef(mLanguages.at(0), desc, desc->get_root()));
        if (auto& compoundcrs = dataSetDef->getCompoundcrs())
        {
          if (std::find(mSupportedCrss.begin(), mSupportedCrss.end(), compoundcrs->getCrs()) ==
              mSupportedCrss.end())
          {
            std::ostringstream msg;
            msg << "Unsupported crs '" << compoundcrs->getCrs() << "' in '" << dataSetDef->getName()
                << "' dataset configuration."
                << " Supported values are:";
            for (auto& crs : mSupportedCrss)
              msg << " '" << crs << "'";
            throw std::runtime_error(msg.str());
          }
        }

        result.push_back(dataSetDef);
      }
      catch (const std::exception& err)
      {
        std::cerr << SmartMet::Spine::log_time_str() << ": error reading data set definition"
                  << " file '" << entry.string() << "'" << std::endl;
        throw;
      }
    }
  }

  return result;
}
}
}
}

/**

@page WCS_PLUGIN_CONFIG WCS Plugin configuration

This document describes entries of SmartMet WCS plugin configuration. Names of mandatory
parameters are shown in bold in the table below

<table border="1">

<tr>
<th>Parameter</th>
<th>Type</th>
<th>Use</th>
<th>Description</th>
</tr>

<tr>
<td>url</td>
<td>string</td>
<td>optional (default @b /wcs )</td>
<td>The directory part of URL for WCS service. @b Note that supported languages will be added to the
provided
    string.</td>
</tr>

<tr>
<td>cacheSize</td>
<td>integer</td>
<td>optional (default 100)</td>
<td>Specifies stored queries response cache size</td>
</tr>

<tr>
<td>cacheTimeConstant</td>
<td>integer</td>
<td>optional (default 60)</td>
<td>Specifies stored queries response cache time constant</td>
</tr>

<tr>
<td>defaultExpiresSeconds</td>
<td>integer</td>
<td>optional (default 60)</td>
<td>Specifies default expires seconds for the Expires header field of a requests
    response. The value can also be used in the Cache-Control header field. For
    most of the code the value has no effect, because on the first hand expires
    seconds is supposed to set in the request level. Use this value as a backup
    in a case of failure. Note that some request use this as a default value
    so if the default value is overrided by a definition, smart choice is a plus.</td>
</tr>

<tr>
<td>@b languages</td>
<td>string or array of strings</td>
<td>mandatory</td>
<td>Specifies supported languages. The first (or only) language specification is treated as the
    default one</td>
</tr>

<tr>
<td>@b getCapabilitiesTemplate</td>
<td>string</td>
<td>mandatory</td>
<td>Specifies CTPP2 template to use for generation of GetCapabilities response. The file is searched
in directory specified in parameter @b templateDir unless absolute path provided. Read in
SmartMet::Plugin::WCS::PluginData::create_template_formatters</td>
</tr>

<tr>
<td>@b exceptionTemplate</td>
<td>string</td>
<td>mandatory</td>
<td>Specifies CTPP2 template to use for generation of error report. The file is searched in
directory specified in parameter @b templateDir unless absolute path provided. Read in
SmartMet::Plugin::WCS::PluginData::create_template_formatters</td>
</tr>

<tr>
<td>@b ctppDumpTemplate</td>
<td>string</td>
<td>mandatory</td>
<td>Specifies CTPP2 template to use for generation of debug format response. The file is searched in
directory specified in parameter @b templateDir unless absolute path provided. Read in
SmartMet::Plugin::WCS::PluginData::create_template_formatters</td>
</tr>

<tr>
<td>@b dataSetDir</td>
<td>string</td>
<td>mandatory</td>
<td>Specifies a dataset directory. Files with .conf suffix are read in from the directory.</td>
</tr>

</table>

 */
