#pragma once

#include <exception>
#include <boost/variant.hpp>
#include <ctpp2/CDT.hpp>
#include <spine/HTTP.h>
#include "WcsException.h"

namespace SmartMet
{
namespace Plugin
{
namespace WCS
{
class PluginData;
class WcsException;

class ErrorResponseGenerator
{
 public:
  enum ProcessingPhaseT
  {
    REQ_PARSING,
    REQ_PROCESSING
  };

  struct ErrorResponse
  {
    SmartMet::Spine::HTTP::Status status;
    std::string response;
    std::string logMessage;
    std::string wcsErrCode;
  };

 public:
  ErrorResponseGenerator(PluginData& pluginData);

  virtual ~ErrorResponseGenerator();
  /**
   *   @brief This procedure must be called in exception context (catch block)
   */
  ErrorResponse createErrorResponse(ProcessingPhaseT phase);

 private:
  CTPP::CDT handleWcsException(const WcsException& err);

  CTPP::CDT handleStdException(const std::exception& err, ProcessingPhaseT phase);

  CTPP::CDT handleUnknownException(ProcessingPhaseT phase);

  std::string getWcsErrCode(ProcessingPhaseT phase);

  void addHttpRequestInfo(CTPP::CDT& hash, const SmartMet::Spine::HTTP::Request& request);

  std::string formatMessage(CTPP::CDT& hash);

  std::string formatLogMessage(CTPP::CDT& hash);

 private:
  PluginData& mPluginData;
};
}
}
}
