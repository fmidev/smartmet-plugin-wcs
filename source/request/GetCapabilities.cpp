#include "request/GetCapabilities.h"
#include "QueryBase.h"
#include "WcsConst.h"
#include "WcsException.h"
#include "xml/XmlUtils.h"
#include <boost/algorithm/string.hpp>
#include <ctpp2/CDT.hpp>
#include <macgyver/StringConversion.h>
#include <macgyver/TypeName.h>

namespace SmartMet
{
namespace Plugin
{
namespace WCS
{
namespace ba = boost::algorithm;
namespace Request
{
GetCapabilities::GetCapabilities(const PluginData& pluginData)
    : RequestBase(),
      mPluginData(pluginData),
      mSupportedFormats({"application/gml+xml; version=3.2",
                         "text/xml; subtype=gml/3.2",
                         "text/xml; version=3.2",
                         "application/gml+xml; subtype=gml/3.2"})
{
}

GetCapabilities::~GetCapabilities() {}
RequestType GetCapabilities::getType() const
{
  return GET_CAPABILITIES;
}
void GetCapabilities::execute(std::ostream& output) const
{
  CTPP::CDT hash;
  const auto& capabilities = mPluginData.getCapabilities();

  auto fmiApikey = getFmiApikey();
  hash["fmi_apikey"] = fmiApikey ? *fmiApikey : "";
  hash["fmi_apikey_prefix"] = QueryBase::FMI_APIKEY_PREFIX_SUBST;
  auto hostname = getHostname();
  hash["hostname"] = hostname ? *hostname : mPluginData.getConfig().getFallbackHostname();
  auto language =
      (not mAcceptLanguages.empty() ? mAcceptLanguages.at(0) : mPluginData.getLanguages().at(0));
  hash["language"] = language;

  int ind = 0;
  for (auto& l : (mAcceptLanguages.empty() ? mPluginData.getLanguages() : mAcceptLanguages))
    hash["languages"][ind++] = l;

  ind = 0;
  for (auto& l : mPluginData.getCswLanguages())
    hash["csw_languages"][ind++] = l;
  hash["csw_language"] = mPluginData.getCswLanguages().at(0);

  hash["version"] = (mResponseVersion ? *mResponseVersion : capabilities.getHighestVersion());
  ind = 0;
  if (mAcceptVersions.empty())
  {
    for (auto& v : capabilities.getSupportedVersions())
      hash["versions"][ind++] = v;
  }
  else
  {
    for (auto& v : mAcceptVersions)
      hash["versions"][ind++] = v;
  }

  for (const std::string& operation : capabilities.getSupportedOperations())
  {
    hash["operation"][operation] = "1";
  }

  ind = 0;
  for (auto& crs : mPluginData.getConfig().getSupportedCrss())
    hash["supported_crss"][ind++] = crs;

  ind = 0;
  for (auto& format : capabilities.getSupportedFormats(RequestType::GET_COVERAGE))
    hash["formats"][ind++] = format;

  int ind2 = 0;
  ind = 0;
  std::set<std::string> checkId;
  for (const auto& mapItem : capabilities.getSupportedDatasets())
  {
    auto dataSetDef = mapItem.second.getDataSetDef();
    CTPP::CDT& f = hash["dataSets"][ind++];
    f["coverage_id"] = mapItem.first;
    f["subtype"] = dataSetDef->getSubtype();
    if (auto title = dataSetDef->getTitle(language))
      f["title"] = title.get();

    if (auto abstract = dataSetDef->getAbstract(language))
      f["abstract"] = abstract.get();

    if (auto keywords = dataSetDef->getKeywords())
    {
      int keyInd = 0;
      for (auto& keyword : keywords.get())
        f["keywords"][keyInd++] = keyword;
    }
    // Collect Inspire Spatial Dataset Identidiers if available
    if (dataSetDef->getDataset() and not checkId.count(dataSetDef->getDataset()->getId()))
    {
      checkId.insert(dataSetDef->getDataset()->getId());
      CTPP::CDT& f2 = hash["dataSetIds"][ind2++];
      auto ds = dataSetDef->getDataset();
      f2["id"] = ds->getId();
      if (auto& datasetUuid = ds->getUuid())
        f2["uuid"] = *datasetUuid;
      if (auto& datasetIdNs = ds->getIdNs())
        f2["ns"] = *datasetIdNs;
    }

    if (auto wgs84BBox = dataSetDef->getWgs84BBox())
    {
      f["wgs84BBox"]["lc"][0] = wgs84BBox->getRangeLat().getMin();
      f["wgs84BBox"]["lc"][1] = wgs84BBox->getRangeLon().getMin();
      f["wgs84BBox"]["uc"][0] = wgs84BBox->getRangeLat().getMax();
      f["wgs84BBox"]["uc"][1] = wgs84BBox->getRangeLon().getMax();
    }
  }

  auto metadataHash = capabilities.getServiceMetaData(language)->getHash();
  if (metadataHash.Size() > 0)
    hash.MergeCDT(metadataHash);

  std::string logMessages;
  try
  {
    auto formatter = mPluginData.getGetCapabilitiesFormatter();
    assert(formatter != 0);
    std::string response;
    formatter->process(hash, response, logMessages);
    substituteAll(response, output);
  }
  catch (const std::exception&)
  {
    std::ostringstream msg;
    msg << METHOD_NAME << ": template formatter exception '" << Fmi::current_exception_type()
        << "' catched: " << logMessages;
    throw WcsException(WcsException::OPERATION_PROCESSING_FAILED, msg.str());
  }
}

boost::shared_ptr<GetCapabilities> GetCapabilities::createFromKvp(
    const Spine::HTTP::Request& httpRequest, const PluginData& pluginData)
{
  boost::shared_ptr<GetCapabilities> request;
  // FIXME: verify required stuff from the request
  request.reset(new GetCapabilities(pluginData));
  request->checkKvpAttributes(httpRequest);

  return request;
}

boost::shared_ptr<GetCapabilities> GetCapabilities::createFromXml(
    const xercesc::DOMDocument& document, const PluginData& pluginData)
{
  boost::shared_ptr<GetCapabilities> request;
  request.reset(new GetCapabilities(pluginData));
  request->checkXmlAttributes(document);

  return request;
}

void GetCapabilities::checkXmlAttributes(const xercesc::DOMDocument& document)
{
  checkRequestName(document, "GetCapabilities");
  checkServiceName(document, "WCS");

  const xercesc::DOMElement* root = RequestBase::getXmlRoot(document);
  std::vector<std::string> versions;
  std::vector<xercesc::DOMElement*> acceptversionsElems =
      Xml::get_child_elements(*root, OWS_NAMESPACE_URI, "AcceptVersions");

  if (not acceptversionsElems.empty())
  {
    std::vector<xercesc::DOMElement*> elems =
        Xml::get_child_elements(*acceptversionsElems.at(0), OWS_NAMESPACE_URI, "Version");

    for (const xercesc::DOMElement* elem : elems)
    {
      versions.push_back(Xml::extract_text(*elem));
    }
    checkVersions(versions);
  }

  std::vector<std::string> languages;
  std::vector<xercesc::DOMElement*> acceptlanguagesElems =
      Xml::get_child_elements(*root, OWS_NAMESPACE_URI, "AcceptLanguages");
  if (not acceptlanguagesElems.empty())
  {
    std::vector<xercesc::DOMElement*> elems =
        Xml::get_child_elements(*acceptlanguagesElems.at(0), OWS_NAMESPACE_URI, "Language");

    for (const xercesc::DOMElement* elem : elems)
    {
      languages.push_back(Xml::extract_text(*elem));
    }
  }
  checkLanguages(mPluginData.getLanguages(), languages);
}

void GetCapabilities::checkKvpAttributes(const Spine::HTTP::Request& httpRequest)
{
  checkRequestName(httpRequest, "GetCapabilities");
  checkServiceName(httpRequest, "WCS");

  auto versionStr = httpRequest.getParameter("acceptversions");
  if (versionStr)
  {
    std::vector<std::string> versions;
    ba::split(versions, *versionStr, ba::is_any_of(","));
    checkVersions(versions);
  }

  std::vector<std::string> languages;
  auto languagesStr = httpRequest.getParameter("AcceptLanguages");
  if (languagesStr)
  {
    ba::split(languages, *languagesStr, ba::is_any_of(","));
  }
  checkLanguages(mPluginData.getLanguages(), languages);
}

void GetCapabilities::checkVersions(const std::vector<std::string>& versions)
{
  if (versions.empty())
    return;

  bool versionOk = false;
  auto& versionAccepted = mPluginData.getCapabilities().getSupportedVersions();
  for (auto& version : versions)
  {
    if (versionAccepted.find(ba::trim_copy(version)) != versionAccepted.end())
    {
      if (not mResponseVersion)
        mResponseVersion = version;
      mAcceptVersions.push_back(version);
      versionOk = true;
    }
  }
  if (not versionOk)
  {
    std::ostringstream msg;
    msg << "Unsupported WCS version(s).";
    WcsException err(WcsException::VERSION_NEGOTIATION_FAILED, msg.str());
    throw err;
  }
}

void GetCapabilities::checkLanguages(const std::vector<std::string>& languagesAccepted,
                                     const std::vector<std::string>& languages)
{
  if (languages.empty())
  {
    mAcceptLanguages = languagesAccepted;
    return;
  }

  const std::string langAnyStr = "*";
  bool languageAnyOk = false;
  for (auto& lang : languages)
  {
    ba::trim_copy(lang);
    if (lang == langAnyStr)
      languageAnyOk = true;

    if (std::find(languagesAccepted.begin(), languagesAccepted.end(), lang) !=
            languagesAccepted.end() or
        languageAnyOk)
    {
      if (mAcceptLanguages.empty() and languageAnyOk)
      {
        mAcceptLanguages = languagesAccepted;
        break;
      }

      if (languageAnyOk)
        break;
      else
        mAcceptLanguages.push_back(lang);
    }
  }
  if (not languageAnyOk)
  {
    std::ostringstream msg;
    msg << "Mandatory parameter value '" << langAnyStr << "' not found.";
    WcsException err(WcsException::MISSING_PARAMETER_VALUE, msg.str());
    err.setLocation("AcceptLanguages");
    throw err;
  }
}

int Request::GetCapabilities::getResponseExpiresSeconds() const
{
  return mPluginData.getConfig().getDefaultExpiresSeconds();
}
}  // namespace Request
}  // namespace WCS
}  // namespace Plugin
}  // namespace SmartMet
