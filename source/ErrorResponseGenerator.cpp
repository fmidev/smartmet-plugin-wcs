#include "ErrorResponseGenerator.h"
#include "PluginData.h"
#include <boost/algorithm/string.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>
#include <boost/format.hpp>
#include <macgyver/TypeName.h>
#include <spine/Convenience.h>

namespace SmartMet
{
namespace Plugin
{
namespace WCS
{
namespace ba = boost::algorithm;
namespace pt = boost::posix_time;

ErrorResponseGenerator::ErrorResponseGenerator(PluginData& pluginData) : mPluginData(pluginData)
{
}

ErrorResponseGenerator::~ErrorResponseGenerator()
{
}
ErrorResponseGenerator::ErrorResponse ErrorResponseGenerator::createErrorResponse(
    ProcessingPhaseT phase)
{
  ErrorResponse response;

  try
  {
    throw;
  }
  catch (const WcsException& err)
  {
    auto hash = handleWcsException(err);
    response.status = err.getStatusCode();
    response.response = formatMessage(hash);
    response.logMessage = formatLogMessage(hash);
    response.wcsErrCode = hash["exceptionList"][0]["exceptionCode"].GetString();
    return response;
  }
  catch (const std::exception& err)
  {
    auto hash = handleStdException(err, phase);
    response.status = SmartMet::Spine::HTTP::bad_request;
    response.response = formatMessage(hash);
    response.logMessage = formatLogMessage(hash);
    response.wcsErrCode = hash["exceptionList"][0]["exceptionCode"].GetString();
    return response;
  }
  catch (...)
  {
    auto hash = handleUnknownException(phase);
    response.status = SmartMet::Spine::HTTP::internal_server_error;
    response.response = formatMessage(hash);
    response.logMessage = formatLogMessage(hash);
    response.wcsErrCode = hash["exceptionList"][0]["exceptionCode"].GetString();
    return response;
  }

  std::cerr << METHOD_NAME << " [INTERNAL ERROR]: is only expected to be called"
            << " from a catch block" << std::endl;
  abort();
}

CTPP::CDT ErrorResponseGenerator::handleWcsException(const WcsException& err)
{
  return err;
}
CTPP::CDT ErrorResponseGenerator::handleStdException(const std::exception& err,
                                                     ProcessingPhaseT phase)
{
  std::ostringstream msg;
  msg << pt::to_simple_string(mPluginData.getLocalTimeStamp()) << " C++ exception '"
      << Fmi::get_type_name(&err) << "': " << err.what();

  CTPP::CDT hash;
  hash["exceptionList"][0]["exceptionCode"] = getWcsErrCode(phase);
  hash["exceptionList"][0]["textList"][0] = msg.str();
  return hash;
}

CTPP::CDT ErrorResponseGenerator::handleUnknownException(ProcessingPhaseT phase)
{
  std::ostringstream msg;
  std::cerr << pt::to_simple_string(mPluginData.getLocalTimeStamp()) << " error: "
            << "Unknown exception '" << Fmi::current_exception_type() << "'";

  CTPP::CDT hash;
  hash["exceptionList"][0]["exceptionCode"] = getWcsErrCode(phase);
  hash["exceptionList"][0]["textList"][0] = msg.str();
  return hash;
}

std::string ErrorResponseGenerator::getWcsErrCode(ProcessingPhaseT phase)
{
  switch (phase)
  {
    case REQ_PARSING:
      return "OperationParsingFailed";

    case REQ_PROCESSING:
    default:
      return "OperationProcessingFailed";
  }
}

void ErrorResponseGenerator::addHttpRequestInfo(CTPP::CDT& hash,
                                                const SmartMet::Spine::HTTP::Request& request)
{
  using boost::format;
  using boost::str;

  const auto method = request.getMethod();
  if (method == Spine::HTTP::RequestMethod::POST)
  {
    auto contentType = request.getHeader("Content-Type");
    auto content = request.getContent();
    hash["exceptionList"][0]["textList"].PushBack(str(format("URI: %1%") % request.getURI()));

    if (contentType)
    {
      hash["exceptionList"][0]["textList"].PushBack(
          str(format("Content-Type: %1%") % *contentType));
    }

    if (content != "")
    {
      hash["exceptionList"][0]["textList"].PushBack(
          str(format("Content: %1%)") % request.getContent()));
    }
  }
  else if (method == Spine::HTTP::RequestMethod::GET)
  {
    hash["exceptionList"][0]["textList"].PushBack(str(format("URI: %1%") % request.getURI()));
  }
}

std::string ErrorResponseGenerator::formatMessage(CTPP::CDT& hash)
{
  std::ostringstream output, logMessages;
  try
  {
    auto exceptionFormatter = mPluginData.getExceptionFormatter();
    exceptionFormatter->get()->process(hash, output, logMessages);
  }
  catch (const std::exception& err)
  {
    std::cerr << METHOD_NAME
              << ": failed to format error message. CTTP2 error messages are:" << logMessages.str()
              << std::endl;
    return "INTERNAL ERROR";
  }
  const std::string content = output.str();
  return content;
}

std::string ErrorResponseGenerator::formatLogMessage(CTPP::CDT& hash)
{
  std::ostringstream msg;
  const std::string content = hash["exceptionList"][0].RecursiveDump(10);
  msg << SmartMet::Spine::log_time_str() << ": ERROR: " << content << std::endl;
  return msg.str();
}
}
}
}
