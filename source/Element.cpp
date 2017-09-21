#include "Element.h"
#include "WcsException.h"
#include "xml/XmlUtils.h"

#include <boost/algorithm/string.hpp>
#include <sstream>

namespace SmartMet
{
namespace Plugin
{
namespace WCS
{
Element::Element(const xercesc::DOMElement* element)
{
  std::pair<std::string, std::string> nameInfo = Xml::get_name_info(element);
  auto value = boost::algorithm::trim_copy(Xml::extract_text(*element));
  if (not value.empty())
  {
    mValue = Value(value);
    mName = nameInfo.first;
  }
  else
  {
    std::ostringstream msg;
    msg << "Empty value detected in '" << nameInfo.first << "'.";
    throw WcsException(WcsException::INVALID_SUBSETTING, msg.str());
  }
}

Element::Element(const Value& value, const NameType& name) : mValue(value), mName(name)
{
  if (mValue == Value())
  {
    std::ostringstream msg;
    msg << "Empty value detected in '" << mName << "'.";
    throw WcsException(WcsException::INVALID_SUBSETTING, msg.str());
  }
}
}
}
}
