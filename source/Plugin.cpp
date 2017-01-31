#include "Plugin.h"
#include <memory>
#include <iostream>
#include <stdexcept>
#include <typeinfo>
#include <cxxabi.h>
#include <boost/algorithm/string.hpp>
#include <boost/bind.hpp>
#include <boost/date_time/gregorian/gregorian.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>
#include <boost/filesystem.hpp>
#include <boost/format.hpp>
#include <boost/lambda/lambda.hpp>
#include <boost/ref.hpp>
#include <xercesc/util/PlatformUtils.hpp>
#include <ctpp2/CDT.hpp>
#include <spine/Convenience.h>
#include <spine/FmiApiKey.h>
#include <spine/Location.h>
#include <spine/TableFormatterOptions.h>
#include <macgyver/TypeName.h>
#include <spine/SmartMet.h>
#include <spine/Reactor.h>
#include <macgyver/TimeFormatter.h>
#include <spine/TableFormatterFactory.h>
#include <macgyver/TypeName.h>
#include "ErrorResponseGenerator.h"
#include "request/GetCapabilities.h"
#include "request/GetCoverage.h"
#include "request/DescribeCoverage.h"
#include "WcsConst.h"
#include "WcsConvenience.h"
#include "WcsException.h"
#include "xml/XmlParser.h"
#include "xml/XmlUtils.h"

using namespace std;
using namespace boost::posix_time;
using namespace boost::gregorian;
namespace ba = boost::algorithm;
namespace bl = boost::lambda;
namespace fs = boost::filesystem;

using boost::format;
using boost::str;

namespace SmartMet
{
namespace Plugin
{
namespace WCS
{
Plugin::Plugin(SmartMet::Spine::Reactor* reactor, const char* config)
    : SmartMetPlugin(), mModuleName("WCS"), mReactor(reactor), mConfig(config)
{
}

void Plugin::init()
{
  try
  {
    mPluginData.reset(new PluginData(mReactor, mConfig));
    mQueryCache.reset(new QueryResponseCache(mPluginData->getConfig().getCacheSize(),
                                             mPluginData->getConfig().getCacheTimeConstant()));
    mRequestFactory.reset(new RequestFactory(*mPluginData));

    if (mReactor->getRequiredAPIVersion() != SMARTMET_API_VERSION)
    {
      std::ostringstream msg;
      msg << "WCS Plugin and Server SmartMet API version mismatch"
          << " (plugin API version is " << SMARTMET_API_VERSION << ", server requires "
          << mReactor->getRequiredAPIVersion() << ")";
      throw std::runtime_error(msg.str());
    }

    mRequestFactory->registerRequestType(
                       "GetCapabilities",
                       RequestType::GET_CAPABILITIES,
                       boost::bind(&Plugin::parseKvpGetCapabilitiesRequest, this, _1),
                       boost::bind(&Plugin::parseXmlGetCapabilitiesRequest, this, _1, _2))
        .registerRequestType("DescribeCoverage",
                             RequestType::DESCRIBE_COVERAGE,
                             boost::bind(&Plugin::parseKvpDescribeCoverageRequest, this, _1),
                             boost::bind(&Plugin::parseXmlDescribeCoverageRequest, this, _1, _2))
        .registerRequestType("GetCoverage",
                             RequestType::GET_COVERAGE,
                             boost::bind(&Plugin::parseKvpGetCoverageRequest, this, _1),
                             boost::bind(&Plugin::parseXmlGetCoverageRequest, this, _1, _2))
        .registerUnimplementedRequestType("DescribeSpatialDataSet")
        .registerUnimplementedRequestType("GetSpatialDataSet");

    const std::string url = mPluginData->getConfig().defaultUrl();
    if (!mReactor->addContentHandler(
            this, url, boost::bind(&Plugin::realRequestHandler, this, _1, _2, _3)))
    {
      std::ostringstream msg;
      msg << "Failed to register WCS content handler";
      throw std::runtime_error(msg.str());
    }
  }
  catch (...)
  {
    throw SmartMet::Spine::Exception(BCP, "Init failed!", NULL);
  }
}

Plugin::~Plugin()
{
}
const std::string& Plugin::getPluginName() const
{
  return mModuleName;
}
int Plugin::getRequiredAPIVersion() const
{
  return SMARTMET_API_VERSION;
}
class Plugin::RequestResult : public Spine::HTTP::ContentStreamer
{
 public:
  std::string getChunk()
  {
    try
    {
      if (not mInitDone)
      {
        mOutput.seekp(0, std::ios_base::end);
        mTotalSize = mOutput.tellp();
        mOutput.seekp(0, std::ios_base::beg);
        mTotalChunks = mTotalSize / mChunkSize;
        mLastChunkSize = mTotalSize % mChunkSize;
        mChunk = 0;
        mCh.resize(mChunkSize);
        setStatus(Spine::HTTP::ContentStreamer::StreamerStatus::OK);
        mInitDone = true;
      }
      else
        mOutput.seekp(mChunkSize, std::ios_base::cur);

      if (mChunk < mTotalChunks)
      {
        mOutput.readsome(&*mCh.begin(), mChunkSize);
      }
      else if (mChunk == mTotalChunks)
      {
        mCh.resize(mLastChunkSize);
        mOutput.readsome(&*mCh.begin(), mLastChunkSize);
      }
      else
      {
        mCh.resize(0);
        setStatus(Spine::HTTP::ContentStreamer::StreamerStatus::EXIT_OK);
      }

      mChunk++;

      return mCh;
    }
    catch (...)
    {
      throw SmartMet::Spine::Exception(BCP, "Init failed!", NULL);
    }
  }

  void setExpiresSeconds(const int& val) { mExpiresSeconds = val; }
  inline const boost::optional<int>& getExpiresSeconds() const { return mExpiresSeconds; }
  void setMayValidateXml(const bool& val) { mMayValidateXml = val; }
  inline const bool& getMayValidateXml() const { return mMayValidateXml; }
  std::stringstream& getOutput() { return mOutput; }
  void setResultStatus(const SmartMet::Spine::HTTP::Status& status) { mStatus = status; }
  inline const SmartMet::Spine::HTTP::Status& getResultStatus() const { return mStatus; }
 private:
  std::stringstream mOutput;
  boost::optional<int> mExpiresSeconds;
  SmartMet::Spine::HTTP::Status mStatus;
  bool mMayValidateXml;
  bool mInitDone;
  size_t mChunkSize;
  size_t mTotalSize;
  size_t mTotalChunks;
  size_t mLastChunkSize;
  size_t mChunk;
  std::string mCh;

 public:
  RequestResult()
      : mOutput(),
        mStatus(SmartMet::Spine::HTTP::not_a_status),
        mMayValidateXml(true),
        mInitDone(false),
        mChunkSize(1440),
        mTotalSize(0),
        mTotalChunks(0),
        mLastChunkSize(0),
        mChunk(0)
  {
  }
};

/**
 *  @brief Perform actual WCS request and generate the response
 */
void Plugin::query(const SmartMet::Spine::HTTP::Request& req, Plugin::RequestResult& result)
{
  const auto method = req.getMethod();

  std::string hostname;
  if (const auto headerXForwardedHost = req.getHeader("X-Forwarded-Host"))
    hostname = *headerXForwardedHost;
  else if (const auto headerHost = req.getHeader("Host"))
    hostname = *headerHost;
  else
    hostname = mPluginData->getConfig().getFallbackHostname();

  const std::string fmiApikeyPrefix = "/fmi-apikey/";
  const std::string language = mPluginData->getLanguages().at(0);

  if (method == Spine::HTTP::RequestMethod::GET)
  {
    boost::shared_ptr<RequestBase> request = mRequestFactory->parseKvp(req);
    request->setHostname(hostname);
    auto fmiApikey = getFmiApikey(req);
    if (fmiApikey)
    {
      request->setFmiApikeyPrefix(fmiApikeyPrefix);
      request->setFmiApikey(*fmiApikey);
    }
    result.setMayValidateXml(request->mayValidateXml());
    request->execute(result.getOutput());
    result.setExpiresSeconds(request->getResponseExpiresSeconds());
    if (request->getHttpStatus())
      result.setResultStatus(request->getHttpStatus());
  }

  else if (method == Spine::HTTP::RequestMethod::POST)
  {
    const std::string contentType = getMandatoryHeader(req, "Content-Type");

    auto fmiApikey = getFmiApikey(req);

    if (contentType == "application/x-www-form-urlencoded")
    {
      boost::shared_ptr<RequestBase> request = mRequestFactory->parseKvp(req);
      request->setHostname(hostname);
      if (fmiApikey)
      {
        request->setFmiApikeyPrefix(fmiApikeyPrefix);
        request->setFmiApikey(*fmiApikey);
      }
      result.setMayValidateXml(request->mayValidateXml());
      request->execute(result.getOutput());
      result.setExpiresSeconds(request->getResponseExpiresSeconds());
      if (request->getHttpStatus())
        result.setResultStatus(request->getHttpStatus());
    }
    else if (contentType == "text/xml")
    {
      const std::string& content = req.getContent();
      if (content == "")
      {
        std::ostringstream msg;
        msg << METHOD_NAME << ": no request content available";
        throw WcsException(WcsException::OPERATION_PARSING_FAILED, msg.str());
      }

      boost::shared_ptr<xercesc::DOMDocument> xmlDoc;
      Xml::Parser::RootElemInfo rootInfo;
      try
      {
        Xml::Parser::root_element_cb_t rootElementCb = (bl::var(rootInfo) = bl::_1);
        xmlDoc = getXmlParser()->parse_string(content, "WCS", rootElementCb);
      }
      catch (const Xml::XmlError& origErr)
      {
        const std::list<std::string>& messages = origErr.get_messages();

        std::ostringstream msg;
        msg << "Failed to parse incoming XML request: " << origErr.what() << "\n";
        WcsException err(WcsException::NO_APPLICABLE_CODE, msg.str());
        err.addText(messages.begin(), messages.end());
        err.setLanguage(language);
        throw err;
      }

      boost::shared_ptr<RequestBase> request = mRequestFactory->parseXml(*xmlDoc);
      request->setHostname(hostname);
      if (fmiApikey)
      {
        request->setFmiApikeyPrefix(fmiApikeyPrefix);
        request->setFmiApikey(*fmiApikey);
      }
      result.setMayValidateXml(request->mayValidateXml());
      request->execute(result.getOutput());
      result.setExpiresSeconds(request->getResponseExpiresSeconds());
      if (request->getHttpStatus())
        result.setResultStatus(request->getHttpStatus());
    }
    else
    {
      std::ostringstream msg;
      msg << METHOD_NAME << ": content type '" << contentType << "' is not supported";
      WcsException err(WcsException::OPERATION_PARSING_FAILED, msg.str());
      err.setLanguage(language);
      throw err;
    }
  }
  else
  {
    std::ostringstream msg;
    msg << METHOD_NAME << ": HTTP method '" << req.getMethodString() << "' is not supported";
    WcsException err(WcsException::OPERATION_PARSING_FAILED, msg.str());
    err.setLanguage(language);
    throw err;
  }
}

void Plugin::requestHandler(SmartMet::Spine::Reactor& reactor,
                            const SmartMet::Spine::HTTP::Request& request,
                            SmartMet::Spine::HTTP::Response& response)
{
  this->realRequestHandler(reactor, request, response);
}

void Plugin::realRequestHandler(SmartMet::Spine::Reactor& /* reactor */,
                                const SmartMet::Spine::HTTP::Request& request,
                                SmartMet::Spine::HTTP::Response& response)
{
  try
  {
    ptime tNow = second_clock::universal_time();

    if (itsShutdownRequested)
      throw WcsException(WcsException::NO_APPLICABLE_CODE, "Service Not available");

    boost::shared_ptr<RequestResult> result(new RequestResult);
    query(request, *result);
    const std::string& content = result->getOutput().str();

    auto status = result->getResultStatus() ? result->getResultStatus() : SmartMet::Spine::HTTP::ok;
    response.setStatus(status);

    // Latter (false) should newer happen.
    const int expiresSeconds = (result->getExpiresSeconds())
                                   ? result->getExpiresSeconds().get()
                                   : mPluginData->getConfig().getDefaultExpiresSeconds();

    // Build cache expiration time info
    ptime tExpires = tNow + seconds(expiresSeconds);

    // The headers themselves
    boost::shared_ptr<Fmi::TimeFormatter> tformat(Fmi::TimeFormatter::create("http"));

    bool CDF = content.substr(0, 3) == "CDF";

    const std::string mime = (CDF ? "application/octet-stream"
                                  : (content.substr(0, 6) == "<html>" ? "text/html; charset=UTF-8"
                                                                      : "text/xml; charset=UTF-8"));
    response.setHeader("Content-Type", mime.c_str());

    if (status == SmartMet::Spine::HTTP::ok)
    {
      std::string cachecontrol =
          "public, max-age=" + boost::lexical_cast<std::string>(expiresSeconds);
      std::string expiration = tformat->format(tExpires);
      std::string modification = tformat->format(tNow);

      response.setHeader("Cache-Control", cachecontrol.c_str());
      response.setHeader("Expires", expiration.c_str());
      response.setHeader("Last-Modified", modification.c_str());
      response.setHeader("Access-Control-Allow-Origin", "*");
    }

    if (CDF)
    {
      response.setHeader("Content-Disposition", "attachement; filename=file.nc");
      response.setContent(result);
    }
    else
    {
      if (result->getMayValidateXml())
        maybeValidateOutput(request, response);
      response.setContent(result->getOutput().str());

      if (response.getContentLength() == 0)
      {
        std::ostringstream msg;
        msg << "Warning: Empty input for request " << request.getQueryString() << " from "
            << request.getClientIP() << std::endl;
        throw WcsException(WcsException::OPERATION_PROCESSING_FAILED, msg.str());
      }
    }
  }
  catch (SmartMet::Spine::Exception& exception)
  {
    WcsException err(WcsException::NO_APPLICABLE_CODE, exception.what());

    response.setContent(err.what());
    response.setStatus(err.getStatusCode());
    response.setHeader("Content-Type", "text/xml; charset=UTF8");
    response.setHeader("Access-Control-Allow-Origin", "*");
    response.setHeader("X-WCS-Error", err.exceptionCodeString());
    std::cerr << err.what();
  }
  catch (...)
  {
    // FIXME: implement correct processing phase support (parsing, processing)
    ErrorResponseGenerator errorResponseGenerator(*mPluginData);
    const auto errorResponse =
        errorResponseGenerator.createErrorResponse(ErrorResponseGenerator::REQ_PROCESSING);
    response.setContent(errorResponse.response);
    response.setStatus(errorResponse.status);
    response.setHeader("Content-Type", "text/xml; charset=UTF8");
    response.setHeader("Access-Control-Allow-Origin", "*");
    response.setHeader("X-WCS-Error", errorResponse.wcsErrCode);
    std::cerr << errorResponse.logMessage;
  }
}

bool Plugin::queryIsFast(const SmartMet::Spine::HTTP::Request& /* theRequest */) const
{
  // FIXME: implement test
  return false;
}

void Plugin::maybeValidateOutput(const SmartMet::Spine::HTTP::Request& req,
                                 SmartMet::Spine::HTTP::Response& response) const
{
  if (mPluginData->getConfig().getValidateXmlOutput())
  {
    try
    {
      const std::string content = response.getContent();
      std::size_t pos = content.find_first_not_of(" \t\r\n");
      if (pos != std::string::npos)
      {
        if (ba::iequals(content.substr(pos, 6), "<html>") or content.substr(pos, 1) != "<")
        {
          return;
        }
      }
      else
      {
        return;
      }

      getXmlParser()->parse_string(content);
    }
    catch (const Xml::XmlError& err)
    {
      std::ostringstream msg;
      msg << SmartMet::Spine::log_time_str()
          << " [WCS] [ERROR] XML Response validation failed: " << err.what() << '\n';
      for (const std::string& err_msg : err.get_messages())
      {
        msg << "       XML: " << err_msg << std::endl;
      }
      const std::string reqStr = req.toString();
      std::vector<std::string> lines;
      ba::split(lines, reqStr, ba::is_any_of("\n"));
      msg << "   WCS request:\n";
      for (const auto& line : lines)
      {
        msg << "       " << ba::trim_right_copy_if(line, ba::is_any_of(" \t\r\n")) << '\n';
      }
      std::cout << msg.str() << std::flush;
    }
  }
#ifdef WCS_DEBUG
  else
  {
    try
    {
      const std::string content = response.getContent();
      if (content.substr(0, 5) == "<?xml")
      {
        Xml::str2xmldom(response.getContent(), "wcs_query_tmp");
      }
    }
    catch (const Xml::XmlError& err)
    {
      std::cout << "\nXML: non validating XML response read failed\n";
      std::cout << "XML: " << err.what() << std::endl;
      for (const std::string& msg : err.get_messages())
      {
        std::cout << "XML: " << msg << std::endl;
      }
    }
  }
#endif
}

boost::optional<std::string> Plugin::getFmiApikey(
    const SmartMet::Spine::HTTP::Request& request) const
{
  return SmartMet::Spine::FmiApiKey::getFmiApiKey(request, true);
}

Xml::Parser* Plugin::getXmlParser() const
{
  return mPluginData->getXmlParser()->get();
}
RequestBaseP Plugin::parseKvpGetCapabilitiesRequest(const SmartMet::Spine::HTTP::Request& request)
{
  return Request::GetCapabilities::createFromKvp(request, *mPluginData);
}

RequestBaseP Plugin::parseXmlGetCapabilitiesRequest(const xercesc::DOMDocument& document,
                                                    const xercesc::DOMElement& root)
{
  (void)root;

  return Request::GetCapabilities::createFromXml(document, *mPluginData);
}

RequestBaseP Plugin::parseKvpDescribeCoverageRequest(const SmartMet::Spine::HTTP::Request& request)
{
  return Request::DescribeCoverage::createFromKvp(request, *mPluginData);
}

RequestBaseP Plugin::parseXmlDescribeCoverageRequest(const xercesc::DOMDocument& document,
                                                     const xercesc::DOMElement& root)
{
  (void)root;

  return Request::DescribeCoverage::createFromXml(document, *mPluginData);
}

RequestBaseP Plugin::parseKvpGetCoverageRequest(const SmartMet::Spine::HTTP::Request& request)
{
  return Request::GetCoverage::createFromKvp(request, *mPluginData, *mQueryCache);
}

RequestBaseP Plugin::parseXmlGetCoverageRequest(const xercesc::DOMDocument& document,
                                                const xercesc::DOMElement& root)
{
  (void)root;
  return Request::GetCoverage::createFromXml(document, *mPluginData, *mQueryCache);
}
}
}
}

/*
 * Server knows us through the 'SmartMetPlugin' virtual interface, which
 * the 'Plugin' class implements.
 */

extern "C" SmartMetPlugin* create(SmartMet::Spine::Reactor* them, const char* config)
{
  return new SmartMet::Plugin::WCS::Plugin(them, config);
}

extern "C" void destroy(SmartMetPlugin* us)
{
  // This will call 'Plugin::~Plugin()' since the destructor is virtual
  delete us;
}

// ======================================================================
