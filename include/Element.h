#pragma once

#include <spine/Value.h>
#include <xercesc/dom/DOMDocument.hpp>

namespace SmartMet
{
namespace Plugin
{
namespace WCS
{
class Element
{
 public:
  using NameType = std::string;
  using Value = Spine::Value;
  Element(const xercesc::DOMElement* element);
  Element(const Value& value, const NameType& name);
  virtual ~Element() {}
  const Value& getValue() const { return mValue; }
  const NameType& getName() const { return mName; }

 private:
  Value mValue;
  NameType mName;
};
}  // namespace WCS
}  // namespace Plugin
}  // namespace SmartMet
