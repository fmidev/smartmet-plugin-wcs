#include "RequestFactory.h"
#include "DescribeCoverage.h"
#include "GetCapabilities.h"
#include "GetCoverage.h"
#include "WcsException.h"
#include <boost/algorithm/string.hpp>
#include <macgyver/StringConversion.h>
#include <macgyver/TypeName.h>
#include <sstream>
#include <stdexcept>

namespace SmartMet
{
namespace Plugin
{
namespace WCS
{
namespace ba = boost::algorithm;

RequestFactory::RequestFactory(PluginData& pluginData) : mPluginData(pluginData)
{
}
RequestFactory::~RequestFactory()
{
}
RequestFactory& RequestFactory::registerRequestType(const std::string& name,
                                                    const RequestType& requestType,
                                                    ParseKvpType createFromKvp,
                                                    ParseXmlType createFromXml)
{
  TypeRec rec;
  const std::string lname = Fmi::ascii_tolower_copy(name);
  rec.kvpParser = createFromKvp;
  rec.xmlParser = createFromXml;

  if (mUnimplementedRequests.count(lname) > 0)
  {
    std::ostringstream msg;
    msg << METHOD_NAME << ": WCS request '" << name << "' already registred as unimplemented";
    throw std::runtime_error(msg.str());
  }

  if (not mTypeMap.insert(std::make_pair(lname, rec)).second)
  {
    std::ostringstream msg;
    msg << METHOD_NAME << ": duplicate WCS request type '" << name << "'";
    throw std::runtime_error(msg.str());
  }

  mRequestNames.insert(name);

  if (requestType == RequestType::GET_CAPABILITIES)
  {
    Request::GetCapabilities gcap(mPluginData);
    for (auto format : gcap.getSupportedFormats())
      mPluginData.getCapabilities().registerOutputFormat(RequestType::GET_CAPABILITIES, format);
  }
  else if (requestType == RequestType::DESCRIBE_COVERAGE)
  {
    Request::DescribeCoverage dcov(mPluginData);
    for (auto format : dcov.getSupportedFormats())
      mPluginData.getCapabilities().registerOutputFormat(RequestType::DESCRIBE_COVERAGE, format);
  }
  else if (requestType == RequestType::GET_COVERAGE)
  {
    Request::GetCoverage gcov(mPluginData);
    for (auto format : gcov.getSupportedFormats())
      mPluginData.getCapabilities().registerOutputFormat(RequestType::GET_COVERAGE, format);
  }
  else
  {
    std::ostringstream msg;
    msg << METHOD_NAME << ": Warning : unsupported WCS request '" << name << "'";
    throw std::runtime_error(msg.str());
  }

  mPluginData.getCapabilities().registerOperation(name);

  return *this;
}

RequestFactory& RequestFactory::registerUnimplementedRequestType(const std::string& name)
{
  const std::string lname = Fmi::ascii_tolower_copy(name);
  if (mTypeMap.count(lname) > 0)
  {
    std::ostringstream msg;
    msg << METHOD_NAME << ": Implementation of WCS request '" << name << "' is already registred";
    throw std::runtime_error(msg.str());
  }

  mRequestNames.insert(name);
  mUnimplementedRequests.insert(Fmi::ascii_tolower_copy(name));
  return *this;
}

boost::shared_ptr<RequestBase> RequestFactory::parseKvp(
    const SmartMet::Spine::HTTP::Request& httpRequest) const
{
  auto service = httpRequest.getParameter("service");
  if (not service)
  {
    WcsException err(WcsException::MISSING_PARAMETER_VALUE, "Missing parameter.");
    err.setLocation("service");
    throw err;
  }
  if (*service != "WCS")
  {
    std::ostringstream msg;
    msg << "Incorrect service '" << *service << "' ('WCS' expected)";
    WcsException err(WcsException::INVALID_PARAMETER_VALUE, msg.str());
    err.setLocation("service");
    throw err;
  }

  const auto name = RequestBase::extractRequestName(httpRequest);
  if (mUnimplementedRequests.count(Fmi::ascii_tolower_copy(name)))
  {
    std::ostringstream msg;
    msg << "WCS request '" << name << "' is not yet implemented";
    WcsException err(WcsException::OPERATION_NOT_SUPPORTED, msg.str());
    err.setLocation("request");
    throw err;
  }

  auto typeRec = getTypeRec(name);
  if (typeRec.kvpParser)
  {
    return (typeRec.kvpParser)(httpRequest);
  }
  else
  {
    std::ostringstream msg;
    msg << "KVP format not supported for WCS request '" << name << "'";
    throw WcsException(WcsException::OPERATION_NOT_SUPPORTED, msg.str());
  }
}

boost::shared_ptr<RequestBase> RequestFactory::parseXml(const xercesc::DOMDocument& document) const
{
  const xercesc::DOMElement* root = document.getDocumentElement();
  if (root == 0)
  {
    std::ostringstream msg;
    msg << "XML root element missing";
    throw WcsException(WcsException::NO_APPLICABLE_CODE, msg.str());
  }

  const auto name = RequestBase::extractRequestName(document);
  if (mUnimplementedRequests.count(Fmi::ascii_tolower_copy(name)))
  {
    std::ostringstream msg;
    msg << "Operation '" << name << "' is not yet implemented";
    WcsException err(WcsException::OPERATION_NOT_SUPPORTED, msg.str());
    err.setLocation("request");
    throw err;
  }

  auto typeRec = getTypeRec(name);
  if (typeRec.xmlParser)
  {
    return (typeRec.xmlParser)(document, *root);
  }
  else
  {
    std::ostringstream msg;
    msg << METHOD_NAME << ": XML format not supported (or not yet supported) for WCS request '"
        << name << "'";
    throw WcsException(WcsException::OPERATION_PARSING_FAILED, msg.str());
  }
}

const RequestFactory::TypeRec& RequestFactory::getTypeRec(const std::string& name) const
{
  const std::string lname = Fmi::ascii_tolower_copy(name);

  auto itemIt = mTypeMap.find(lname);
  if (itemIt == mTypeMap.end())
  {
    std::ostringstream msg;
    msg << "Unrecognized WCS request '" << name << "'";
    WcsException err(WcsException::INVALID_PARAMETER_VALUE, msg.str());
    err.setLocation("request");
    throw err;
  }
  return itemIt->second;
}
}
}
}
