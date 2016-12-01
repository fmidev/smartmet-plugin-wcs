#pragma once

#include <map>
#include <string>
#include <boost/function.hpp>
#include <boost/shared_ptr.hpp>
#include <xercesc/dom/DOMDocument.hpp>
#include <spine/HTTP.h>
#include "PluginData.h"
#include "RequestBase.h"

namespace SmartMet
{
namespace Plugin
{
namespace WCS
{
/**
 *   @brief Factory class for creating apropriate WCS request objects from
 *          KVP and XML format WCS requests
 */
class RequestFactory
{
 public:
  using ParseKvpType =
      boost::function1<boost::shared_ptr<RequestBase>, const SmartMet::Spine::HTTP::Request&>;

  using ParseXmlType = boost::function2<boost::shared_ptr<RequestBase>,
                                        const xercesc::DOMDocument&,
                                        const xercesc::DOMElement&>;

 public:
  RequestFactory(PluginData& pluginData);

  virtual ~RequestFactory();

  /**
   *   @brief Registers factory methods for creating WCS request objects
   *
   *   @param name the name of the request
   *   @param requestType A RequestType id.
   *   @param createFromKvp factory method for creating the request object from
   *          HTTP format KVP request. KVP is not supported if empty boost::function1
   *          provided.
   *   @param createFromXml factory method for creating the request object from
   *          HTTP format XML request. XML is not supported if empty boost::function1
   *          provided.
   */
  RequestFactory& registerRequestType(const std::string& name,
                                      const RequestType& requestType,
                                      ParseKvpType createFromKvp,
                                      ParseXmlType createFromXml);

  /**
   *   @brief Registers name of unimplemented WCS request
   */
  RequestFactory& registerUnimplementedRequestType(const std::string& name);

  /**
   *   @brief Creates WCS request object from KVP request
   *
   *   This method is for
   *   - HTTP GET when request parameters are provided inside request URL
   *   - HTTP POST with content type application/x-www-form-urlencoded when request
   *     parameters are provided in HTTP request body.
   */
  boost::shared_ptr<RequestBase> parseKvp(const SmartMet::Spine::HTTP::Request& httpRequest) const;

  /**
   *   @brief Creates WCS request object from XML format request
   *
   *   Note: SOAP requests are not yet supported
   */
  boost::shared_ptr<RequestBase> parseXml(const xercesc::DOMDocument& document) const;

  /**
   *   @brief Check whether provided request name is valid (although possibly unimplemented)
   *
   *   Warning: check is case sensitive in this case
   */
  inline bool checkRequestName(const std::string& name) const
  {
    return mRequestNames.count(name) > 0;
  }

 private:
  struct TypeRec
  {
    ParseKvpType kvpParser;
    ParseXmlType xmlParser;
  };

  const TypeRec& getTypeRec(const std::string& name) const;

  std::set<std::string> mRequestNames;
  std::map<std::string, TypeRec> mTypeMap;
  std::set<std::string> mUnimplementedRequests;

  PluginData& mPluginData;
};
}
}
}
