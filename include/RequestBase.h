#pragma once

#include "RequestType.h"
#include <string>
#include <set>
#include <xercesc/dom/DOMDocument.hpp>
#include <xercesc/dom/DOMElement.hpp>
#include <spine/HTTP.h>

namespace SmartMet
{
namespace Plugin
{
namespace WCS
{
/**
 *   @brief Abstract base class for WCS requests
 */
class RequestBase
{
 public:
  using FormatType = std::string;
  using SupportedFormatsType = std::vector<FormatType>;
  using CoverageIdType = std::string;
  using CoverageIdsType = std::vector<CoverageIdType>;

 public:
  RequestBase();

  virtual ~RequestBase();

  void setIsSoapRequest(bool value);

  inline bool isSoapRequest() const { return mSoapRequest; }
  /**
   *   @brief Get seconds after which response is considered expire.
   *
   *   The expire value is meant to use in the Expires header
   *   of a response (e.g. the max-age cache-control directive).
   *   For new reques types defaultExpiresSeconds can be used from
   *   the main configuration.
   *   @return Seconds as an integer.
   */
  virtual int getResponseExpiresSeconds() const = 0;

  /**
   *   @brief Get type of the request
   */
  virtual RequestType getType() const = 0;

  /**
   *   @brief Execute the request and return the result as the
   *          string in the case of a success.
   */
  virtual void execute(std::ostream& ost) const = 0;

  /**
   *   @brief Perform dynamic cast to corresponding request type
   *
   *   Throws std::bad_cast if dynamic_cast fails
   */
  template <typename RequestTypeName>
  RequestTypeName* cast()
  {
    RequestTypeName& result = dynamic_cast<RequestTypeName&>(*this);
    return &result;
  }

  /**
   *   @brief Extract the request name and namespace URI from DOM document
   *
   *   The first element of returned pair contains the name and the second one -
   *   namespace uri.
   */
  static std::string extractRequestName(const xercesc::DOMDocument& document);

  /**
   *   @brief Extract the request name and namespace URI from KVP request
   */
  static std::string extractRequestName(const SmartMet::Spine::HTTP::Request& httpRequest);

  /**
   *   @brief Specifies whether response XML may be validated
   *
   *   Note that response XML validation may be disabled in configuration.
   *   The result of this virtual method only allows or disallow
   *   validation if it is already enabled in configuration
   */
  virtual bool mayValidateXml() const;

  virtual void setFmiApikey(const std::string& fmiApikey);

  virtual void setFmiApikeyPrefix(const std::string& fmiApikeyPrefix);

  virtual void setHostname(const std::string& hostname);

  virtual const SupportedFormatsType& getSupportedFormats() const = 0;

  boost::optional<std::string> getFmiApikey() const { return mFmiApikey; }
  boost::optional<std::string> getHostname() const { return mHostname; }
  void substituteAll(const std::string& src, std::ostream& output) const;

  void substituteFmiApikey(const std::string& src, std::ostream& output) const;

  void substituteHostname(const std::string& src, std::ostream& output) const;

  void setHttpStatus(SmartMet::Spine::HTTP::Status status) const;

  inline SmartMet::Spine::HTTP::Status getHttpStatus() const { return mStatus; }
 protected:
  /**
   *   @brief Verifies that a parameter name from HTTP request is correct
   */
  static void checkParameterName(const SmartMet::Spine::HTTP::Request& request,
                                 const std::string& parameter,
                                 const std::string& name);

  /**
   *   @brief Verifies that the request name from HTTP request is correct
   */
  static void checkRequestName(const SmartMet::Spine::HTTP::Request& request,
                               const std::string& name);

  /**
   *   @brief Verifies that the request name from XML request is correct
   */
  static void checkRequestName(const xercesc::DOMDocument& document, const std::string& name);

  /**
   *   @brief Verifies that the service name from HTTP request is correct
   */
  static void checkServiceName(const SmartMet::Spine::HTTP::Request& request,
                               const std::string& name);

  /**
   *   @brief Verifies that the service name from XML request is correct
   */
  static void checkServiceName(const xercesc::DOMDocument& document, const std::string& name);

  /**
   *   @brief Verifies presence and values of mandatory attributes of root element
   */
  virtual void checkXmlAttributes(const xercesc::DOMDocument& document);
  virtual void checkKvpAttributes(const SmartMet::Spine::HTTP::Request& request);

  /**
   *   @brief Get DOM document root (here only to have error checking in one place)
   */
  static const xercesc::DOMElement* getXmlRoot(const xercesc::DOMDocument& document);

  static void checkOutputFormatAttribute(const std::string& value);

  /**
   *   @brief Check WCS version from HTTP request if provided.
   */
  static void checkWcsVersion(const SmartMet::Spine::HTTP::Request& request,
                              const std::set<std::string>& versionsAccepted);

  /**
   *   @brief Check WCS version from XML request if provided.
   */
  static void checkWcsVersion(const xercesc::DOMDocument& document,
                              const std::set<std::string>& versionsAccepted);

 private:
  /**
   *   Specifies whether the request is generated from SOAP request
   */
  bool mSoapRequest;

  boost::optional<std::string> mFmiApikey;
  boost::optional<std::string> mFmiApikeyPrefix;
  boost::optional<std::string> mHostname;

  mutable SmartMet::Spine::HTTP::Status mStatus;
};

using RequestBaseP = boost::shared_ptr<RequestBase>;
}
}
}
