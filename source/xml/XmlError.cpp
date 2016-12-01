#include "xml/XmlError.h"
#include <boost/format.hpp>
#include <macgyver/TypeName.h>

namespace SmartMet
{
namespace Plugin
{
namespace WCS
{
namespace Xml
{
XmlError::XmlError(const std::string& text, error_level_t error_level = XmlError::ERROR)
    : std::runtime_error(text), error_level(error_level)
{
}

XmlError::~XmlError() throw()
{
}
void XmlError::add_messages(const std::list<std::string>& messages)
{
  this->messages = messages;
}

const char* XmlError::error_level_name(enum error_level_t error_level)
{
  using boost::str;
  using boost::format;
  switch (error_level)
  {
    case ERROR:
      return "ERROR";
    case FATAL_ERROR:
      return "FATAL_ERROR";
    default:
      throw std::runtime_error(str(format("XmlError: unknown error level %2%") % (int)error_level));
  }
}
}
}
}
}
