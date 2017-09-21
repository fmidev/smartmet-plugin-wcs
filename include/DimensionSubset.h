#pragma once

#include "Element.h"

#include <engines/gis/CRSRegistry.h>
#include <engines/querydata/Producer.h>
#include <xercesc/dom/DOMDocument.hpp>

namespace SmartMet
{
namespace Plugin
{
namespace WCS
{
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
