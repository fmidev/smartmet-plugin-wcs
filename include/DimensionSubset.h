#pragma once

#include "Abbreviations.h"
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
  using ChildrenNameSet = std::set<NameType>;

  DimensionSubset(const ChildrenNameSet& allowedChildrens = {"Dimension"});
  virtual ~DimensionSubset();

  // Allowed axis labels
  void setAbbreviations(const Abbreviations::Shared& abbreviations);

  Value get(const NameType& name) const;
  virtual void setSubset(const xercesc::DOMElement* element);
  virtual void setSubset(const std::string& subset);

 protected:
  const Abbreviations::Shared& getAbbreviations() const;
  void setElement(const Element&& element);

 private:
  std::map<NameType, Element> mElements;
  ChildrenNameSet mAllowedChildrens;
  Abbreviations::Shared mAbbreviations;
};

class DimensionTrim : public DimensionSubset
{
 public:
  DimensionTrim();
  ~DimensionTrim();

  void setSubset(const xercesc::DOMElement* element);
  void setSubset(const std::string& ss);
};

class DimensionSlice : public DimensionSubset
{
 public:
  DimensionSlice();
  ~DimensionSlice();

  void setSubset(const xercesc::DOMElement* element);
  void setSubset(const std::string& subset);
};
}
}
}
