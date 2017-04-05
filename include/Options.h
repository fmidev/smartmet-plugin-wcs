#pragma once

#include "DimensionSubset.h"
#include "OptionsBase.h"
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
class Options : public OptionsBase
{
 public:
  using Shared = std::shared_ptr<Options>;
  using NameType = std::string;
  using DimensionSliceMapType = std::map<NameType, DimensionSlice>;
  using DimensionTrimMapType = std::map<NameType, DimensionTrim>;
  using OutputCrsType = std::string;
  using ProducerType = SmartMet::Engine::Querydata::Producer;
  using ParameterNameType = std::string;
  using Transformation = SmartMet::Engine::Gis::CRSRegistry::Transformation;
  using SwapType = bool;

  Options() : mSwap(false) {}
  ~Options() {}
  void setDimensionSubset(const DimensionSubset& dimensionSubset);
  void setOutputCrs(const OutputCrsType& outputCrs) { mOutputCrs = outputCrs; }
  void setProducer(const ProducerType& producer) { mProducer = producer; }
  void setParameterName(const ParameterNameType& parameterName) { mParameterName = parameterName; }
  void setTransformation(boost::shared_ptr<Transformation> transformation)
  {
    mTransformation = transformation;
  }
  void setSwap(const SwapType& swap) { mSwap = swap; }
  boost::optional<DimensionSlice> getDimensionSlice(const NameType& name) const;
  boost::optional<DimensionTrim> getDimensionTrim(const NameType& name) const;
  boost::optional<OutputCrsType> getOutputCrs() const { return mOutputCrs; }
  const ProducerType& getProducer() const { return mProducer; }
  const ParameterNameType& getParameterName() const { return mParameterName; }
  boost::shared_ptr<Transformation> getTransformation() const { return mTransformation; }
  SwapType getSwap() const { return mSwap; }

 protected:
 private:
  Options(const Options& other) = delete;
  Options& operator=(const Options& other) = delete;
  DimensionSliceMapType mDimensionSlices;
  DimensionTrimMapType mDimensionTrims;
  boost::optional<OutputCrsType> mOutputCrs;
  ProducerType mProducer;
  ParameterNameType mParameterName;
  boost::shared_ptr<Transformation> mTransformation;
  SwapType mSwap;
};
}
}
}
