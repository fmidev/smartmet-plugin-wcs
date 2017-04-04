#include "RequestBase.h"
#include "QueryBase.h"
#include "WcsConst.h"
#include "WcsException.h"
#include "xml/XmlUtils.h"
#include <boost/algorithm/string.hpp>
#include <macgyver/StringConversion.h>
#include <macgyver/TypeName.h>
#include <sstream>

namespace SmartMet
{
namespace Plugin
{
namespace WCS
{
namespace ba = boost::algorithm;

RequestBase::RequestBase() : mSoapRequest(false), mStatus(SmartMet::Spine::HTTP::not_a_status)
{
}
RequestBase::~RequestBase()
{
}
void RequestBase::setIsSoapRequest(bool value)
{
  mSoapRequest = value;
}
std::string RequestBase::extractRequestName(const xercesc::DOMDocument& document)
{
  const xercesc::DOMElement* root = getXmlRoot(document);
  auto nameInfo = Xml::get_name_info(root);
  if (nameInfo.second == SOAP_NAMESPACE_URI)
  {
    std::ostringstream msg;
    msg << "HTTP/SOAP requests are not yet supported";
    throw std::runtime_error(msg.str());
  }
  else if (nameInfo.second == WCS_NAMESPACE_URI)
  {
    if (ba::trim_copy(nameInfo.first).empty())
    {
      WcsException err(WcsException::MISSING_PARAMETER_VALUE,
                       "Operation does not include a paramer value.");
      err.setLocation("request");
      throw err;
    }

    return nameInfo.first;
  }
  else
  {
    if (nameInfo.second == "")
    {
      std::ostringstream msg;
      msg << "No namespace provided but " << WCS_NAMESPACE_URI << " expected.";
      WcsException err(WcsException::NO_APPLICABLE_CODE, msg.str());
      throw err;
    }
    else
    {
      std::ostringstream msg;
      msg << " Unexpected XML namespace " << nameInfo.second << " when " << WCS_NAMESPACE_URI
          << " expected.";
      WcsException err(WcsException::NO_APPLICABLE_CODE, msg.str());
      throw err;
    }
  }

  return nameInfo.first;
}

std::string RequestBase::extractRequestName(const SmartMet::Spine::HTTP::Request& httpRequest)
{
  auto reqName = httpRequest.getParameter("request");
  if (not reqName)
  {
    WcsException err(WcsException::MISSING_PARAMETER_VALUE,
                     "Request parameter and a value is missing.");
    err.setLocation("request");
    throw err;
  }

  if (ba::trim_copy(*reqName).empty())
  {
    WcsException err(WcsException::MISSING_PARAMETER_VALUE,
                     "Operation does not include a parameter value.");
    err.setLocation("request");
    throw err;
  }

  return *reqName;
}

bool RequestBase::mayValidateXml() const
{
  return true;
}
void RequestBase::setFmiApikey(const std::string& fmiApikey)
{
  this->mFmiApikey = fmiApikey;
}

void RequestBase::setFmiApikeyPrefix(const std::string& fmiApikeyPrefix)
{
  this->mFmiApikeyPrefix = fmiApikeyPrefix;
}

void RequestBase::setHostname(const std::string& hostname)
{
  this->mHostname = hostname;
}
void RequestBase::substituteAll(const std::string& src, std::ostream& output) const
{
  std::ostringstream tmp1;
  substituteFmiApikey(src, tmp1);
  std::ostringstream tmp2;
  substituteHostname(tmp1.str(), output);
}

void RequestBase::substituteFmiApikey(const std::string& src, std::ostream& output) const
{
  std::ostringstream tmpOut;
  ba::replace_all_copy(std::ostreambuf_iterator<char>(tmpOut),
                       src,
                       std::string(QueryBase::FMI_APIKEY_PREFIX_SUBST),
                       mFmiApikeyPrefix ? *mFmiApikeyPrefix : std::string(""));

  ba::replace_all_copy(std::ostreambuf_iterator<char>(output),
                       tmpOut.str(),
                       std::string(QueryBase::FMI_APIKEY_SUBST),
                       mFmiApikey ? *mFmiApikey : std::string(""));
}

void RequestBase::substituteHostname(const std::string& src, std::ostream& output) const
{
  ba::replace_all_copy(std::ostreambuf_iterator<char>(output),
                       src,
                       std::string(QueryBase::HOSTNAME_SUBST),
                       mHostname ? *mHostname : std::string("localhost"));
}

void RequestBase::setHttpStatus(SmartMet::Spine::HTTP::Status status) const
{
  this->mStatus = status;
}

void RequestBase::checkParameterName(const SmartMet::Spine::HTTP::Request& httpRequest,
                                     const std::string& param,
                                     const std::string& name)
{
  auto paramName = httpRequest.getParameter(param);
  if (not paramName)
  {
    WcsException err(WcsException::MISSING_PARAMETER_VALUE, "Parameter and a value is missing.");
    err.setLocation(param);
    throw err;
  }

  if (ba::trim_copy(*paramName).empty())
  {
    WcsException err(WcsException::MISSING_PARAMETER_VALUE,
                     "Operation does not include a parameter value.");
    err.setLocation(param);
    throw err;
  }

  // Parameter values are case sensitive (according to the OGC specifications),
  // though servers usually honour case-insensitive requests
  if (Fmi::ascii_tolower_copy(*paramName) != Fmi::ascii_tolower_copy(name))
  {
    WcsException err(WcsException::INVALID_PARAMETER_VALUE,
                     "Operation contains an invalid parameter value.");
    err.setLocation(param);
    throw err;
  }
}

void RequestBase::checkRequestName(const SmartMet::Spine::HTTP::Request& httpRequest,
                                   const std::string& name)
{
  checkParameterName(httpRequest, "request", name);
}

void RequestBase::checkRequestName(const xercesc::DOMDocument& document, const std::string& name)
{
  const std::string actualName = extractRequestName(document);
  if (Fmi::ascii_tolower_copy(actualName) != Fmi::ascii_tolower_copy(name))
  {
    WcsException err(WcsException::INVALID_PARAMETER_VALUE,
                     "Operation contains an invalid parameter value.");
    err.setLocation("request");
    throw err;
  }
}

void RequestBase::checkServiceName(const SmartMet::Spine::HTTP::Request& httpRequest,
                                   const std::string& name)
{
  checkParameterName(httpRequest, "service", name);
}

void RequestBase::checkServiceName(const xercesc::DOMDocument& document, const std::string& name)
{
  const xercesc::DOMElement* root = getXmlRoot(document);
  try
  {
    Xml::verify_mandatory_attr_value(*root, WCS_NAMESPACE_URI, "service", name);
  }
  catch (const std::exception& err)
  {
    WcsException err(WcsException::INVALID_PARAMETER_VALUE,
                     "Operation contains an invalid parameter value.");
    err.setLocation("service");
    throw err;
  }
}

void RequestBase::checkXmlAttributes(const xercesc::DOMDocument& document)
{
}
void RequestBase::checkKvpAttributes(const SmartMet::Spine::HTTP::Request& request)
{
}
void RequestBase::checkOutputFormatAttribute(const std::string& value)
{
  namespace ba = boost::algorithm;

  std::vector<std::string> w;
  ba::split(w, value, ba::is_any_of(";"));
  if ((w.size() != 2) or (Fmi::ascii_tolower_copy(ba::trim_copy(w[0])) != "application/gml+xml") or
      (Fmi::ascii_tolower_copy(ba::trim_copy(w[1])) != "version=3.2"))
  {
    std::ostringstream msg;
    msg << "Unsupported output format '" << value
        << "' ('application/gml+xml; version=3.2' expected)";
    throw WcsException(WcsException::OPERATION_PARSING_FAILED, msg.str());
  }
}

const xercesc::DOMElement* RequestBase::getXmlRoot(const xercesc::DOMDocument& document)
{
  xercesc::DOMElement* root = document.getDocumentElement();
  if (root == NULL)
  {
    std::ostringstream msg;
    throw WcsException(
        WcsException::OPERATION_PARSING_FAILED,
        "SmartMet::Plugin::Wcs::RequestBase::get_xml_root(): XML root element missing");
  }
  return root;
}

void RequestBase::checkWcsVersion(const SmartMet::Spine::HTTP::Request& request,
                                  const std::set<std::string>& versionsAccepted)
{
  auto version = request.getParameter("version");
  if (version)
  {
    if (versionsAccepted.find(*version) == versionsAccepted.end())
    {
      std::ostringstream msg;
      msg << "Incorrect WCS version '" << *version << "'";
      throw WcsException(WcsException::INVALID_PARAMETER_VALUE, msg.str());
    }
  }
}

void RequestBase::checkWcsVersion(const xercesc::DOMDocument& document,
                                  const std::set<std::string>& versionsAccepted)
{
  const xercesc::DOMElement* root = getXmlRoot(document);
  auto version = Xml::get_optional_attr(*root, WCS_NAMESPACE_URI, "version", "");
  if (not version.empty())
  {
    if (versionsAccepted.find(version) == versionsAccepted.end())
    {
      std::ostringstream msg;
      msg << "Incorrect WCS version '" << version << "'";
      throw WcsException(WcsException::INVALID_PARAMETER_VALUE, msg.str());
    }
  }
}
}
}
}
