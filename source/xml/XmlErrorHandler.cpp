#include "xml/XmlErrorHandler.h"
#include "xml/XmlError.h"
#include "xml/XmlUtils.h"
#include <boost/format.hpp>
#include <sstream>

namespace SmartMet
{
namespace Plugin
{
namespace WCS
{
namespace Xml
{
XmlErrorHandler::XmlErrorHandler(bool throw_on_error) : throw_on_error(throw_on_error)
{
  resetErrors();
}

XmlErrorHandler::~XmlErrorHandler() {}
void XmlErrorHandler::warning(const xercesc::SAXParseException& exc)
{
  add_message("WARNING", exc);
  num_warnings++;
}

void XmlErrorHandler::error(const xercesc::SAXParseException& exc)
{
  add_message("ERROR", exc);
  num_errors++;
  check_terminate("Error", XmlError::ERROR);
}

void XmlErrorHandler::fatalError(const xercesc::SAXParseException& exc)
{
  add_message("FATAL ERROR", exc);
  num_fatal_errors++;
  check_terminate("Fatal error", XmlError::FATAL_ERROR);
}

void XmlErrorHandler::resetErrors()
{
  num_warnings = num_errors = num_fatal_errors = 0;
  messages.clear();
}

void XmlErrorHandler::check_errors(const std::string& text)
{
  using boost::format;
  using boost::str;
  if (not isOk())
  {
    XmlError error(text, haveFatalErrors() ? XmlError::FATAL_ERROR : XmlError::ERROR);
    error.add_messages(get_messages());
    throw error;
  }
}

void XmlErrorHandler::add_message(const std::string& prefix, const xercesc::SAXParseException& exc)
{
  std::ostringstream msg;
  msg << prefix << ": " << to_opt_string(exc.getMessage()).first << " at " << exc.getLineNumber()
      << ':' << exc.getColumnNumber() << ", publicId='" << to_opt_string(exc.getPublicId()).first
      << "', systemId='" << to_opt_string(exc.getSystemId()).first << "'";
  messages.push_back(msg.str());
}

void XmlErrorHandler::check_terminate(const std::string& prefix,
                                      XmlError::error_level_t error_level)
{
  if (throw_on_error)
  {
    throw XmlError(prefix + " while parsing XML input. Parsing terminated", error_level);
  }
}
}  // namespace Xml
}  // namespace WCS
}  // namespace Plugin
}  // namespace SmartMet
