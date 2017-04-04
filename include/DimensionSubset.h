#pragma once

#include <engines/gis/CRSRegistry.h>
#include <engines/querydata/Producer.h>
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

class DimensionSubset
{
 public:
  using NameType = Element::NameType;
  using Value = Element::Value;
  DimensionSubset(const xercesc::DOMElement* element,
                  const std::set<NameType>& allowedChildrens = {"Dimension"});
  DimensionSubset(const std::string& subset);
  virtual ~DimensionSubset() {}
  Value get(const NameType& name) const;

 protected:
  DimensionSubset() {}
  void setElement(const Element&& element);

 private:
  std::map<NameType, Element> mElements;
};

class DimensionTrim : public DimensionSubset
{
 public:
  DimensionTrim() {}
  DimensionTrim(const xercesc::DOMElement* element);
  DimensionTrim(const std::string& subset);
};

class DimensionSlice : public DimensionSubset
{
 public:
  DimensionSlice() {}
  DimensionSlice(const xercesc::DOMElement* element);
  DimensionSlice(const std::string& subset);
};
}
}
}
