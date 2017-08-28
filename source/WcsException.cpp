#include "WcsException.h"
#include "WcsConvenience.h"
#include <boost/algorithm/string.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>
#include <boost/optional/optional_io.hpp>

namespace SmartMet
{
namespace Plugin
{
namespace WCS
{
WcsException::WcsException(WcsException::ExceptionCode exceptionCode, const std::string& text)
    : mExceptionCode(exceptionCode),
      mWhatString(text),
      mTextStrings(),
      mStatusCode(SmartMet::Spine::HTTP::bad_request)
{
  switch (mExceptionCode)
  {
    case MISSING_PARAMETER_VALUE:
    case OPERATION_PARSING_FAILED:
    case OPERATION_PROCESSING_FAILED:
    case INVALID_PARAMETER_VALUE:
    case VERSION_NEGOTIATION_FAILED:
    case INVALID_UPDATE_SEQUENCE:
    case EMPTY_COVERAGE_ID_LIST:
    case INVALID_AXIS_LABEL:
    case INVALID_SUBSETTING:
    case NO_SUCH_COVERAGE:
      mStatusCode = SmartMet::Spine::HTTP::bad_request;
      break;
    case OPERATION_NOT_SUPPORTED:
    case OPTION_NOT_SUPPORTED:
      mStatusCode = SmartMet::Spine::HTTP::not_implemented;
      break;
    case NO_APPLICABLE_CODE:
      mStatusCode = SmartMet::Spine::HTTP::internal_server_error;
      break;

    default:
      mStatusCode = SmartMet::Spine::HTTP::bad_request;
  }
}

WcsException::~WcsException() throw()
{
}
std::string WcsException::exceptionCodeString() const
{
  switch (mExceptionCode)
  {
    case MISSING_PARAMETER_VALUE:
      return "MissingParameterValue";

    case OPERATION_NOT_SUPPORTED:
      return "OperationNotSupported";

    case OPERATION_PARSING_FAILED:
      return "OperationParsingFailed";

    case OPERATION_PROCESSING_FAILED:
      return "OperationProcessingFailed";

    case OPTION_NOT_SUPPORTED:
      return "OptionNotSupported";

    case INVALID_PARAMETER_VALUE:
      return "InvalidParameterValue";

    case VERSION_NEGOTIATION_FAILED:
      return "VersionNegotiationFailed";

    case INVALID_UPDATE_SEQUENCE:
      return "InvalidUpdateSequence";

    case NO_APPLICABLE_CODE:
      return "NoApplicableCode";

    case EMPTY_COVERAGE_ID_LIST:
      return "EmptyCoverageIdList";

    case INVALID_AXIS_LABEL:
      return "InvalidAxisLabel";

    case INVALID_SUBSETTING:
      return "InvalidSubsetting";

    case NO_SUCH_COVERAGE:
      return "NoSuchCoverage";

    default:
      return "InvalidExceptionCode";
  }
}

const char* WcsException::what() const throw()
{
  return mWhatString.c_str();
}
void WcsException::addText(const std::string& text)
{
  mTextStrings.push_back(text);
}
WcsException::operator CTPP::CDT() const
{
  CTPP::CDT hash;

  if (mLanguage)
    hash["language"] = *mLanguage;

  hash["exceptionList"][0]["exceptionCode"] = exceptionCodeString();

  if (mLocation)
    hash["exceptionList"][0]["location"] = *mLocation;

  hash["exceptionList"][0]["textList"][0] = mWhatString;
  for (std::size_t i = 0; i < mTextStrings.size(); i++)
  {
    hash["exceptionList"][0]["textList"][i + 1] = mTextStrings[i];
  }

  return hash;
}

void WcsException::printOn(std::ostream& stream) const
{
  namespace ba = boost::algorithm;
  const auto now = mTimestamp ? mTimestamp : boost::posix_time::second_clock::local_time();
  std::ostringstream msg;
  msg << now << ": " << exceptionCodeString() << ": "
      << ba::trim_right_copy_if(mWhatString, ba::is_any_of(" \t\r\n")) << "\n";
  for (std::size_t i = 0; i < mTextStrings.size(); i++)
  {
    const std::string& text = mTextStrings[i];
    if (text.length() > 0)
    {
      msg << now << ":>\t" << ba::trim_right_copy_if(mTextStrings[i], ba::is_any_of(" \t\r\n"))
          << "\n";
    }
  }
  stream << msg.str() << std::flush;
}

void WcsException::setTimestamp(const boost::posix_time::ptime& timestamp)
{
  mTimestamp = timestamp;
}
}
}
}
