#include "request/GetCoverage.h"
#include "ErrorResponseGenerator.h"
#include "Options.h"
#include "WcsConst.h"
#include "WcsException.h"
#include "response/Netcdf4ClassicResponse.h"
#include "xml/XmlUtils.h"
#include <boost/date_time/posix_time/posix_time.hpp>
#include <boost/format.hpp>
#include <boost/lexical_cast.hpp>
#include <macgyver/StringConversion.h>
#include <macgyver/TypeName.h>
#include <spine/Convenience.h>
#include <xercesc/dom/DOM.hpp>
#include <xercesc/dom/DOMException.hpp>
#include <xercesc/dom/DOMXPathNSResolver.hpp>
#include <xercesc/dom/DOMXPathResult.hpp>
#include <xercesc/framework/MemBufInputSource.hpp>
#include <xercesc/util/Janitor.hpp>
#include <xqilla/xqilla-dom3.hpp>
#include <set>

namespace SmartMet
{
namespace Plugin
{
namespace WCS
{
namespace ba = boost::algorithm;
using boost::str;
using boost::format;
namespace Request
{
GetCoverage::GetCoverage(PluginData& pluginData)
    : RequestBase(),
      mOptions(new Options),
      mPluginData(pluginData),
      mSupportedFormats({"application/netcdf"})
{
  if (not mSupportedFormats.size())
    throw std::runtime_error("GetCoverage request does not support any output format");
}

GetCoverage::~GetCoverage()
{
}
RequestType GetCoverage::getType() const
{
  return GET_COVERAGE;
}
void GetCoverage::execute(std::ostream& output) const
{
  int debugLevel = mPluginData.getConfig().getDebugLevel();

  const auto& capabilities = mPluginData.getCapabilities();
  const auto& dataSetMap = capabilities.getSupportedDatasets();
  WcsCapabilities::DatasetMap::const_iterator it = dataSetMap.find(mCoverageIds.at(0));
  if (it == dataSetMap.end())
  {
    WcsException wcsException(WcsException::NO_SUCH_COVERAGE,
                              "Identifier passed does not match with any of the "
                              "coverages offered by this server.");
    throw wcsException;
  }

  // WCS_1-RC1 says:
  // If you make a GetCoverage request and don’t specify any
  // output format the range set will be delivered in XML.
  // However for large gridded data sets XML is not appropriate,
  // so individual INSPIRE data specifications or profiles may
  // make support of one of more output formats an additional
  // conformance requirement.
  if (mOutputFormat)
  {
    if (*mOutputFormat == "application/netcdf")
    {
      Netcdf4ClassicResponse ncSample;
      ncSample.setDebugLevel(debugLevel);
      ncSample.setOptions(mOptions);
      ncSample.setEngine(mPluginData.getQuerydataEngine());
      ncSample.setParamConfig(mPluginData.getNetcdfParamConfig());
      ncSample.get(output);
    }
    /*
    else if (*mOutputFormat == "application/xml")
    {
      executeSingleQuery(output);
    }
    */
    else
    {
      WcsException wcsException(WcsException::INVALID_PARAMETER_VALUE, "Unsupported output format");
      wcsException.addText(*mOutputFormat);
      wcsException.setLocation("format");
      throw wcsException;
    }
  }
  else
  {
    WcsException wcsException(WcsException::MISSING_PARAMETER_VALUE,
                              "Output format is not defined.");
    std::ostringstream msg;
    msg << "Supported formats are:";
    for (const auto& f : mSupportedFormats)
      msg << " '" << f << "'";
    wcsException.addText(msg.str());
    wcsException.setLocation("format");

    throw wcsException;
  }
}

int GetCoverage::getResponseExpiresSeconds() const
{
  return mPluginData.getConfig().getDefaultExpiresSeconds();
}

void GetCoverage::executeSingleQuery(std::ostream& ost) const
{
  CTPP::CDT hash;
  hash["language"] = mPluginData.getLanguages().at(0);
  auto fmiApikey = getFmiApikey();
  hash["fmi_apikey"] = fmiApikey ? *fmiApikey : "";
  hash["fmi_apikey_prefix"] = QueryBase::FMI_APIKEY_PREFIX_SUBST;
  auto hostname = getHostname();
  hash["hostname"] = hostname ? *hostname : mPluginData.getConfig().getFallbackHostname();

  std::ostringstream logMessages;
  try
  {
    auto formatter = mPluginData.getGetCoverageFormater();
    assert(formatter != 0);
    std::ostringstream response;
    formatter->get()->process(hash, response, logMessages);
    substituteAll(response.str(), ost);
  }
  catch (const std::exception&)
  {
    std::ostringstream msg;
    msg << METHOD_NAME << ": template formatter exception '" << Fmi::current_exception_type()
        << "' catched: " << logMessages.str();
    throw WcsException(WcsException::OPERATION_PROCESSING_FAILED, msg.str());
  }
}

boost::shared_ptr<GetCoverage> GetCoverage::createFromKvp(const Spine::HTTP::Request& httpRequest,
                                                          PluginData& pluginData,
                                                          QueryResponseCache& queryCache)
{
  auto ids = httpRequest.getParameterList("coverageid");
  for (auto& id : ids)
  {
    ba::trim(id);
  }
  auto format = httpRequest.getParameter("format");
  auto subsets = httpRequest.getParameterList("subset");
  auto slices = httpRequest.getParameterList("DimensionSlice");
  subsets.insert(subsets.end(), slices.begin(), slices.end());
  auto trims = httpRequest.getParameterList("DimensionTrim");
  subsets.insert(subsets.end(), trims.begin(), trims.end());

  boost::shared_ptr<GetCoverage> getcoverage(new GetCoverage(pluginData));
  getcoverage->checkKvpAttributes(httpRequest);

  if (format)
    getcoverage->setOutputFormat(ba::trim_copy(*format));

  for (auto& ss : subsets)
  {
    DimensionTrim trim(ss);
    if (not trim.get("Dimension").is_empty())
    {
      getcoverage->getOptions()->setDimensionSubset(std::move(trim));
      continue;
    }

    DimensionSlice slice(ss);
    if (not slice.get("Dimension").is_empty())
    {
      getcoverage->getOptions()->setDimensionSubset(std::move(slice));
      continue;
    }

    std::ostringstream msg;
    msg << "Invalid subset '" << ss << "'.";
    throw WcsException(WcsException::INVALID_SUBSETTING, msg.str());
  }

  auto outputCrs = httpRequest.getParameter("outputCrs");
  if (outputCrs)
    getcoverage->getOptions()->setOutputCrs(ba::trim_copy(*outputCrs));

  getcoverage->checkCoverageIds(ids);
  getcoverage->setCoverageIds(ids);

  return getcoverage;
}

void GetCoverage::checkKvpAttributes(const Spine::HTTP::Request& httpRequest)
{
  checkRequestName(httpRequest, "GetCoverage");
  checkServiceName(httpRequest, "WCS");
  checkWcsVersion(httpRequest, mPluginData.getCapabilities().getSupportedVersions());
}

boost::shared_ptr<GetCoverage> GetCoverage::createFromXml(const xercesc::DOMDocument& document,
                                                          PluginData& pluginData,
                                                          QueryResponseCache& queryCache)
{
  const xercesc::DOMElement* root = getXmlRoot(document);

  const char* methodName = "SmartMet::Plugin::Wcs::Request::GetCoverage::create_fromXml";
  boost::shared_ptr<GetCoverage> getcoverage(new GetCoverage(pluginData));
  getcoverage->setQueryCache(queryCache);
  getcoverage->checkXmlAttributes(document);

  std::set<std::string> allowedChildren;
  allowedChildren.insert("Extension");
  allowedChildren.insert("CoverageId");
  allowedChildren.insert("DimensionSubset");
  allowedChildren.insert("DimensionSlice");
  allowedChildren.insert("DimensionTrim");
  allowedChildren.insert("format");
  allowedChildren.insert("mediatype");

  std::vector<std::string> ids;

  auto children = Xml::get_child_elements(*root, WCS_NAMESPACE_URI, "*");
  for (const xercesc::DOMElement* child : children)
  {
    const std::string name =
        Xml::check_name_info(child, WCS_NAMESPACE_URI, allowedChildren, methodName);

    if (name == "CoverageId")
    {
      ids.push_back(ba::trim_copy(Xml::extract_text(*child)));
    }
    else if (name == "DimensionSubset")
    {
    }
    else if (name == "DimensionSlice")
    {
      DimensionSlice slice(child);
      getcoverage->getOptions()->setDimensionSubset(std::move(slice));
    }
    else if (name == "DimensionTrim")
    {
      DimensionTrim trim(child);
      getcoverage->getOptions()->setDimensionSubset(std::move(trim));
    }
    else if (name == "format")
    {
      auto format = ba::trim_copy(Xml::extract_text(*child));
      if (not format.empty())
        getcoverage->setOutputFormat(format);
    }
    else if (name == "mediatype")
    {
    }
    else if (name == "Extension")
    {
      std::set<std::string> allowedExtChildren;
      allowedExtChildren.insert("outputCrs");
      auto extensions = Xml::get_child_elements(*child, OWS_NAMESPACE_URI, "*");
      for (const xercesc::DOMElement* ext : extensions)
      {
        const std::string extName =
            Xml::check_name_info(ext, OWS_NAMESPACE_URI, allowedExtChildren, methodName);
        if (extName == "outputCrs")
        {
          auto outputCrs = ba::trim_copy(Xml::extract_text(*ext));
          getcoverage->getOptions()->setOutputCrs(outputCrs);
        }
      }
    }
    else
    {
      // Not supposed to be here
      assert(0);
    }
  }

  getcoverage->checkCoverageIds(ids);
  getcoverage->setCoverageIds(ids);

  return getcoverage;
}

void GetCoverage::checkXmlAttributes(const xercesc::DOMDocument& document)
{
  checkRequestName(document, "GetCoverage");
  checkServiceName(document, "WCS");
  checkWcsVersion(document, mPluginData.getCapabilities().getSupportedVersions());
}

void GetCoverage::checkCoverageIds(const std::vector<std::string>& ids)
{
  if (ids.empty())
  {
    WcsException err(WcsException::EMPTY_COVERAGE_ID_LIST, "Mandatory parameter value not found.");
    err.setLocation("CoverageId");
    throw err;
  }

  if (ids.size() > 1)
  {
    WcsException err(WcsException::INVALID_PARAMETER_VALUE, "Only one CoverageId allowed");
    err.setLocation("CoverageId");
    throw err;
  }

  std::string idLower = Fmi::ascii_tolower_copy(ids.at(0));

  auto dataSetMap = mPluginData.getCapabilities().getSupportedDatasets();
  if (idLower.empty() or dataSetMap.find(idLower) == dataSetMap.end())
  {
    WcsException wcsException(WcsException::NO_SUCH_COVERAGE,
                              "Identifier passed does not match with any of the "
                              "coverages offered by this server.");
    wcsException.addText(idLower);

    throw wcsException;
  }
}

void GetCoverage::setCoverageIds(const CoverageIdsType& ids)
{
  mCoverageIds = ids;
  std::sort(mCoverageIds.begin(), mCoverageIds.end());

  const auto& capabilities = mPluginData.getCapabilities();
  const auto& dataSetMap = capabilities.getSupportedDatasets();
  WcsCapabilities::DatasetMap::const_iterator it = dataSetMap.find(mCoverageIds.at(0));
  if (it == dataSetMap.end())
  {
    WcsException wcsException(WcsException::NO_SUCH_COVERAGE,
                              "Identifier passed does not match with any of the "
                              "coverages offered by this server.");
    throw wcsException;
  }

  mOptions->setProducer(it->second.getProducer());
  mOptions->setParameterName(it->second.getParameterName());

  // Create SRS transformation
  auto& defaultCrs = mPluginData.getConfig().getDefaultCrs();
  const boost::optional<CompoundCRS>& compoundcrs = it->second.getDataSetDef()->getCompoundcrs();
  std::string crsName = (compoundcrs ? compoundcrs->getCrs() : defaultCrs);

  if (auto outputCrs = mOptions->getOutputCrs())
    crsName = *outputCrs;

  auto& crs_registry = mPluginData.getCrsRegistry();
  auto transformation = crs_registry.create_transformation(defaultCrs, crsName);
  mOptions->setTransformation(transformation);

  bool swapCoord = false;
  crs_registry.get_attribute(crsName, "swapCoord", &swapCoord);
  mOptions->setSwap(swapCoord);
}

void GetCoverage::setOutputFormat(const RequestBase::FormatType& format)
{
  if (std::find(std::begin(mSupportedFormats), std::end(mSupportedFormats), format) ==
      std::end(mSupportedFormats))
  {
    std::ostringstream msg;
    msg << "Unsupported output format value. Supported formats are:";
    for (auto& format : mSupportedFormats)
      msg << " '" << format << "'";
    WcsException err(WcsException::INVALID_PARAMETER_VALUE, msg.str());
    err.setLocation("format");
    throw err;
  }

  mOutputFormat = format;
}
}
}
}
}
