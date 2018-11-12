#include "request/DescribeCoverage.h"
#include "QueryBase.h"
#include "WcsConst.h"
#include "WcsConvenience.h"
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
namespace pt = boost::posix_time;
namespace Request
{
DescribeCoverage::DescribeCoverage(const PluginData& pluginData)
    : RequestBase(),
      mPluginData(pluginData),
      mSupportedFormats({"application/gml+xml; version=3.2",
                         "text/xml; subtype=gml/3.2",
                         "text/xml; version=3.2",
                         "application/gml+xml; subtype=gml/3.2"})
{
}

void DescribeCoverage::setCoverageIds(const std::vector<std::string>& ids)
{
  mIds = ids;
  std::sort(mIds.begin(), mIds.end());
}

DescribeCoverage::~DescribeCoverage() {}
RequestType DescribeCoverage::getType() const
{
  return DESCRIBE_COVERAGE;
}
void DescribeCoverage::execute(std::ostream& output) const
{
  int debugLevel = mPluginData.getConfig().getDebugLevel();

  CTPP::CDT hash;
  const auto& capabilities = mPluginData.getCapabilities();
  const auto& dataSetMap = capabilities.getSupportedDatasets();

  auto fmiApikey = getFmiApikey();
  hash["fmi_apikey"] = fmiApikey ? *fmiApikey : "";
  hash["fmi_apikey_prefix"] = QueryBase::FMI_APIKEY_PREFIX_SUBST;
  auto hostname = getHostname();
  hash["hostname"] = hostname ? *hostname : mPluginData.getConfig().getFallbackHostname();

  int cnt = 0;
  std::set<std::string> validIds;
  std::vector<std::string> invalidIds;
  for (const auto& id : mIds)
  {
    std::string idLower = Fmi::ascii_tolower_copy(id);
    WcsCapabilities::DatasetMap::const_iterator it = dataSetMap.find(idLower);
    if (it == dataSetMap.end())
      invalidIds.push_back(id);
    else
      validIds.insert(idLower);
  }

  if (not invalidIds.empty())
  {
    WcsException wcsException(WcsException::NO_SUCH_COVERAGE,
                              "One of the identifiers passed does not match with any of the "
                              "coverages offered by this server.");
    for (const auto& id : invalidIds)
      wcsException.addText(id);

    throw wcsException;
  }

  const auto querydataEngine = mPluginData.getQuerydataEngine();
  auto& crs_registry = mPluginData.getCrsRegistry();

  for (const auto& id : validIds)
  {
    WcsCapabilities::DatasetMap::const_iterator it = dataSetMap.find(id);
    if (it == dataSetMap.end())
    {
      if (debugLevel > 2)
        std::cout << "Id '" << id << "' skipped\n";
      continue;
    }

    Engine::Querydata::MetaQueryOptions opt;
    opt.setProducer(it->second.getProducer());
    opt.addParameter(it->second.getParameterName());
    std::list<Engine::Querydata::MetaData> metaList = querydataEngine->getEngineMetadata(opt);
    if (metaList.empty())
    {
      std::ostringstream msg;
      msg << "ERROR: Server internal error - "
          << "Meta data not found with the values: producer='" << it->second.getProducer()
          << "' and parameterName='" << it->second.getParameterName() << "'";
      std::cerr << msg.str();
      continue;
    }

    // Skipping the missing Area or Grid cases.
    auto q = querydataEngine->get(it->second.getProducer());
    if (not q->isArea() and q->isGrid())
    {
      if (debugLevel > 2)
        std::cout << "Id '" << id << "' has missing Area or Grid\n";
      continue;
    }
    CTPP::CDT& cov = hash["coverageList"][cnt++];

    bool showHeight = false;
    int crsCode = 0;
    std::string projUri;
    std::string projEpochUri;
    std::string projPressEpochUri;
    std::string projLabels;
    bool swapCoord = false;

    // Get target CRS information from GisEngine.
    const CompoundCRS& compoundcrs = it->second.getDataSetDef()->getCompoundcrs();
    std::string crsName = compoundcrs.getIdentifier();
    crs_registry.get_attribute(crsName, "epsg", &crsCode);
    crs_registry.get_attribute(crsName, "showHeight", &showHeight);
    crs_registry.get_attribute(crsName, "projUri", &projUri);
    crs_registry.get_attribute(crsName, "projEpochUri", &projEpochUri);
    crs_registry.get_attribute(crsName, "projPressEpochUri", &projPressEpochUri);
    crs_registry.get_attribute(crsName, "swapCoord", &swapCoord);

    // EPSG crs code is either 2D or 3D. Compound height is not allowed in the case of 3D.
    unsigned srsDimension = (showHeight ? 3 : 2);
    std::string axisLabels = (swapCoord ? "y x" : "x y");

    if (showHeight)
      axisLabels.append(" z");

    auto abbreviations = compoundcrs.getAbbreviations();
    for (const auto& label : abbreviations->getAbbreviationVector())
    {
      if (not projLabels.empty())
        projLabels.append(" ");
      projLabels.append(label);
    }

    const bool showCompoundcrsHeight = (compoundcrs.getVerticalCRS() ? true : false);
    const bool showCompoundcrsTime = (compoundcrs.getTemporalCRS() ? true : false);

    // Compound height is not allowed in the case of 3D.
    if (showHeight and showCompoundcrsHeight)
    {
      std::ostringstream msg;
      msg << "3D Coordinate Reference System and VerticalCRS of "
          << "compound CRS is not possible to use at the same time."
          << "Check coverageid '" << id << "'configuration. Vertical CRS is '"
          << compoundcrs.getVerticalCRS()->getIdentifier() << "'.";
      WcsException wcsException(WcsException::NO_APPLICABLE_CODE, msg.str());
      throw wcsException;
    }

    unsigned compoundAxisId = 1;
    // Constructing compound CRS URI.
    std::string compoundcrsUri = mPluginData.getConfig().getCompoundcrsUri() + "?";
    compoundcrsUri.append(Fmi::to_string(compoundAxisId++)).append("=").append(crsName);
    if (showCompoundcrsHeight)
    {
      auto& verticalCRS = compoundcrs.getVerticalCRS();
      compoundcrsUri.append("&amp;")
          .append(Fmi::to_string(compoundAxisId++))
          .append("=")
          .append(verticalCRS->getIdentifier());
      axisLabels.append(" z");
      srsDimension++;
    }
    if (showCompoundcrsTime)
    {
      auto& temporalCRS = compoundcrs.getTemporalCRS();
      compoundcrsUri.append("&amp;").append(Fmi::to_string(compoundAxisId++)).append("=");
      compoundcrsUri.append(temporalCRS->getIdentifier());
      axisLabels.append(" t");
      srsDimension++;
    }

    cov["id"] = id;
    cov["subtype"] = it->second.getDataSetDef()->getSubtype();
    cov["fieldName"] = it->second.getParameterName();
    cov["srsName"] = projUri;
    cov["srsNameEpoch"] = projEpochUri;
    cov["srsNamePressEpoch"] = compoundcrsUri;
    cov["srsLabels"] = projLabels;
    cov["axisLabels"] = axisLabels;
    // cov["uomLabels"] = "deg deg";
    cov["srsDimension"] = srsDimension;

    const NFmiArea& area = q->area();

    auto transformation = crs_registry.create_transformation(DATA_CRS_NAME, crsName);
    if (!transformation)
      throw std::runtime_error(
          "Failed to create the needed coordinate transformation for generating positions");

    q->resetLevel();
    q->nextLevel();

    // Define the grid geometry. Coordinate order in the area object
    // is always east-north, so the values are swapped to get correct
    // input order for the tranformation. Output axis order is defined
    // in a crs configuration.
    const NFmiPoint topLeft = transformation->transform(swap(area.TopLeftLatLon(), true));
    const NFmiPoint topRight = transformation->transform(swap(area.TopRightLatLon(), true));
    const NFmiPoint bottomLeft = transformation->transform(swap(area.BottomLeftLatLon(), true));
    const NFmiPoint bottomRight = transformation->transform(swap(area.BottomRightLatLon(), true));

    cov["bottom_left"][0] = bottomLeft.X();
    cov["bottom_left"][1] = bottomLeft.Y();
    cov["top_right"][0] = topRight.X();
    cov["top_right"][1] = topRight.Y();

    const NFmiGrid& grid = q->grid();
    cov["grid_size"][0] = (swapCoord ? grid.YNumber() - 1 : grid.XNumber() - 1);
    cov["grid_size"][1] = (swapCoord ? grid.XNumber() - 1 : grid.YNumber() - 1);

    // If the value is not set the template does not use the dimension.
    if (showHeight or showCompoundcrsHeight)
    {
      cov["grid_size"][2] = 0;
      cov["grid_offset_z"] = 0;
    }
    if (showCompoundcrsTime)
    {
      cov["grid_size"][3] = (metaList.front().nTimeSteps - 1);
      cov["grid_offset_t"] = 60 * metaList.front().timeStep;
    }

    unsigned long maxXInd = grid.XNumber() - 1;
    unsigned long maxYInd = grid.YNumber() - 1;
    unsigned long maxInd = grid.XNumber() * grid.YNumber() - 1;
    cov["max_x_ind"] = maxXInd;
    cov["max_y_ind"] = maxYInd;
    cov["max_ind"] = maxInd;

    double blTlX = (topLeft.X() - bottomLeft.X()) / maxYInd;
    double blTlY = (topLeft.Y() - bottomLeft.Y()) / maxYInd;
    double blBrX = (bottomRight.X() - bottomLeft.X()) / maxXInd;
    double blBrY = (bottomRight.Y() - bottomLeft.Y()) / maxXInd;

    cov["grid_offset_x"][0] = (swapCoord ? blTlX : blBrX);
    cov["grid_offset_x"][1] = (swapCoord ? blTlY : blBrY);
    cov["grid_offset_y"][0] = (swapCoord ? blBrX : blTlX);
    cov["grid_offset_y"][1] = (swapCoord ? blBrY : blTlY);

    cov["bbox_bl"][0] = bottomLeft.X();
    cov["bbox_bl"][1] = bottomLeft.Y();
    cov["bbox_tr"][0] = topRight.X();
    cov["bbox_tr"][1] = topRight.Y();

    int gmCnt = 0;
    q->resetTime();
    q->nextTime();
    cov["bbox_time"][0] = epochToString(q->validTime().PosixTime(), "iso-8901");
    cov["epoch_time"][0] = epochToString(q->validTime().PosixTime());
    q->lastTime();
    cov["bbox_time"][1] = epochToString(q->validTime().PosixTime(), "iso-8901");
    cov["epoch_time"][1] = epochToString(q->validTime().PosixTime());

    double levelValue = static_cast<double>(q->level().LevelValue());
    double levelValueMin = levelValue;
    double levelValueMax = levelValue;
    do
    {
      CTPP::CDT& gmhash = cov["gm"][gmCnt++];
      levelValue = q->level().LevelValue();
      gmhash["level"] = levelValue;
      if (levelValueMin > levelValue)
        levelValueMin = levelValue;
      if (levelValueMax < levelValue)
        levelValueMax = levelValue;
    } while (q->nextLevel());
    cov["elevation"][0] = levelValueMin;
    cov["elevation"][1] = levelValueMax;
  }

  hash["language"] =
      (mAcceptLanguages.size() ? mAcceptLanguages.at(0) : mPluginData.getLanguages().at(0));

  std::ostringstream logMessages;
  try
  {
    auto formatter = mPluginData.getDescribeCoverageFormatter();
    assert(formatter != 0);
    std::ostringstream response;
    formatter->process(hash, response, logMessages);
    substituteAll(response.str(), output);
  }
  catch (const std::exception&)
  {
    std::ostringstream msg;
    msg << METHOD_NAME << ": template formatter exception '" << Fmi::current_exception_type()
        << "' catched: " << logMessages.str();
    throw WcsException(WcsException::OPERATION_PROCESSING_FAILED, msg.str());
  }
}

boost::shared_ptr<DescribeCoverage> DescribeCoverage::createFromKvp(
    const Spine::HTTP::Request& httpRequest, const PluginData& pluginData)
{
  auto ids = httpRequest.getParameterList("coverageid");
  for (auto& id : ids)
  {
    ba::trim(id);
  }

  boost::shared_ptr<DescribeCoverage> request;
  request.reset(new DescribeCoverage(pluginData));
  request->setCoverageIds(ids);
  request->checkKvpAttributes(httpRequest);
  request->checkCoverageIds();
  request->checkKvpAcceptLanguages(httpRequest);

  return request;
}

boost::shared_ptr<DescribeCoverage> DescribeCoverage::createFromXml(
    const xercesc::DOMDocument& document, const PluginData& pluginData)
{
  const xercesc::DOMElement* root = RequestBase::getXmlRoot(document);
  std::vector<std::string> ids;
  std::vector<xercesc::DOMElement*> elems =
      Xml::get_child_elements(*root, WCS_NAMESPACE_URI, "CoverageId");

  for (const xercesc::DOMElement* elem : elems)
  {
    const std::string value = ba::trim_copy(Xml::extract_text(*elem));
    if (not value.empty())
    {
      ids.push_back(value);
    }
  }

  boost::shared_ptr<DescribeCoverage> request;
  request.reset(new DescribeCoverage(pluginData));
  request->setCoverageIds(ids);
  request->checkXmlAttributes(document);
  request->checkCoverageIds();
  request->checkXmlAcceptLanguages(document);

  return request;
}

void DescribeCoverage::checkKvpAttributes(const Spine::HTTP::Request& httpRequest)
{
  checkRequestName(httpRequest, "DescribeCoverage");
  checkServiceName(httpRequest, "WCS");
  checkWcsVersion(httpRequest, mPluginData.getCapabilities().getSupportedVersions());
}
void DescribeCoverage::checkKvpAcceptLanguages(const Spine::HTTP::Request& httpRequest)
{
  std::vector<std::string> languages;
  auto languagesStr = httpRequest.getParameter("AcceptLanguages");
  if (languagesStr)
  {
    ba::split(languages, *languagesStr, ba::is_any_of(","));
  }
  checkLanguages(mPluginData.getLanguages(), languages);
}

void DescribeCoverage::checkXmlAttributes(const xercesc::DOMDocument& document)
{
  checkRequestName(document, "DescribeCoverage");
  checkServiceName(document, "WCS");
  checkWcsVersion(document, mPluginData.getCapabilities().getSupportedVersions());
}

void DescribeCoverage::checkXmlAcceptLanguages(const xercesc::DOMDocument& document)
{
  std::vector<std::string> languages;
  const xercesc::DOMElement* root = RequestBase::getXmlRoot(document);
  std::vector<xercesc::DOMElement*> extensionElems =
      Xml::get_child_elements(*root, WCS_NAMESPACE_URI, "Extension");
  if (not extensionElems.empty())
  {
    std::vector<xercesc::DOMElement*> acceptlanguagesElems =
        Xml::get_child_elements(*extensionElems.at(0), OWS_NAMESPACE_URI, "AcceptLanguages");
    if (not acceptlanguagesElems.empty())
    {
      std::vector<xercesc::DOMElement*> elems =
          Xml::get_child_elements(*acceptlanguagesElems.at(0), OWS_NAMESPACE_URI, "Language");

      for (const xercesc::DOMElement* elem : elems)
      {
        languages.push_back(Xml::extract_text(*elem));
      }
    }
  }
  checkLanguages(mPluginData.getLanguages(), languages);
}

void DescribeCoverage::checkCoverageIds()
{
  if (mIds.empty())
  {
    WcsException err(WcsException::EMPTY_COVERAGE_ID_LIST, "Mandatory parameter value not found.");
    err.setLocation("CoverageId");
    throw err;
  }
}

void DescribeCoverage::checkLanguages(const std::vector<std::string>& languagesAccepted,
                                      const std::vector<std::string>& languages)
{
  if (languages.empty())
  {
    mAcceptLanguages = languagesAccepted;
    return;
  }

  const std::string langAnyStr = "any";
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

int DescribeCoverage::getResponseExpiresSeconds() const
{
  return mPluginData.getConfig().getDefaultExpiresSeconds();
}

NFmiPoint DescribeCoverage::transform(
    const std::unique_ptr<OGRCoordinateTransformation>& transformation, const NFmiPoint&& p) const
{
  double x = p.X();
  double y = p.Y();
  if (not transformation->Transform(1, &x, &y))
  {
    std::ostringstream msg;
    msg << "Transformation failed from '" << transformation->GetSourceCS()->GetEPSGGeogCS()
        << "' to '" << transformation->GetTargetCS()->GetEPSGGeogCS() << "'.";
    WcsException err(WcsException::OPERATION_PROCESSING_FAILED, msg.str());
    throw err;
  }

  return NFmiPoint(x, y);
}

std::string DescribeCoverage::epochToString(const pt::ptime&& epoch,
                                            const std::string&& encodingStyle) const
{
  if (encodingStyle == "iso-8901")
    return Fmi::to_iso_extended_string(epoch) + "Z";
  else
  {
    static const long refJd = boost::gregorian::date(1970, 1, 1).julian_day();
    long long jd = epoch.date().julian_day();
    long seconds = epoch.time_of_day().total_seconds();
    INT_64 sEpoch = 86400LL * (jd - refJd) + seconds;
    return Fmi::to_string(sEpoch);
  }
}
}  // namespace Request
}  // namespace WCS
}  // namespace Plugin
}  // namespace SmartMet
