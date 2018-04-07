#pragma once

#include <boost/date_time/posix_time/ptime.hpp>
#include <boost/optional.hpp>
#include <ctpp2/CDT.hpp>
#include <spine/HTTP.h>
#include <exception>
#include <ostream>
#include <string>
#include <vector>

namespace SmartMet
{
namespace Plugin
{
namespace WCS
{
class WcsException : public std::exception
{
 public:
  enum ExceptionCode
  {
    MISSING_PARAMETER_VALUE,
    OPERATION_NOT_SUPPORTED,
    OPERATION_PARSING_FAILED,
    OPERATION_PROCESSING_FAILED,
    OPTION_NOT_SUPPORTED,
    INVALID_PARAMETER_VALUE,
    VERSION_NEGOTIATION_FAILED,
    INVALID_UPDATE_SEQUENCE,
    NO_APPLICABLE_CODE,
    EMPTY_COVERAGE_ID_LIST,
    INVALID_AXIS_LABEL,
    INVALID_SUBSETTING,
    NO_SUCH_COVERAGE
  };

 public:
  WcsException(ExceptionCode exceptionCode, const std::string& text);
  virtual ~WcsException() throw();

  std::string exceptionCodeString() const;

  virtual const char* what() const throw();

  void addText(const std::string& text);

  operator CTPP::CDT() const;

  inline ExceptionCode getExceptionCode() const { return mExceptionCode; }
  inline const std::vector<std::string>& getTextStrings() const { return mTextStrings; }
  inline const SmartMet::Spine::HTTP::Status& getStatusCode() const { return mStatusCode; }
  inline void setLanguage(const std::string& language) { mLanguage = language; }
  inline void setLocation(const std::string& location) { mLocation = location; }
  template <typename Iterator>
  void addText(Iterator begin, Iterator end)
  {
    while (begin != end)
      addText(*begin++);
  }

  void printOn(std::ostream& output) const;

  void setTimestamp(const boost::posix_time::ptime& timestamp);

 private:
  ExceptionCode mExceptionCode;
  std::string mWhatString;
  std::vector<std::string> mTextStrings;
  boost::optional<std::string> mLanguage;
  boost::optional<std::string> mLocation;
  SmartMet::Spine::HTTP::Status mStatusCode;
  boost::optional<boost::posix_time::ptime> mTimestamp;
};

inline std::ostream& operator<<(std::ostream& output, const WcsException& err)
{
  err.printOn(output);
  return output;
}
}  // namespace WCS
}  // namespace Plugin
}  // namespace SmartMet
