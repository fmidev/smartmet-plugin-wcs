#pragma once

#include "DataContainer.h"
#include "ParamConfig.h"
#include "ResponseBase.h"
#include <boost/optional/optional.hpp>
#include <engines/querydata/Engine.h>
#include <cstddef>
#include <cstdio>
#include <memory>
#include <netcdfcpp.h>
#include <string>

namespace SmartMet
{
namespace Plugin
{
namespace WCS
{
class NetcdfResponse : public ResponseBase
{
 public:
  using DebugLevelType = int;
  using Id = unsigned long;
  using ValueX = double;
  using ValueY = double;
  using ValueZ = double;
  using ValueT = boost::posix_time::ptime;
  using IdValueXPair = std::pair<Id, ValueX>;
  using IdValueYPair = std::pair<Id, ValueY>;
  using IdValueZPair = std::pair<Id, ValueZ>;
  using IdValueTPair = std::pair<Id, ValueT>;
  using RangeT = std::pair<ValueT, ValueT>;
  using RangeZ = std::pair<ValueZ, ValueZ>;
  using QdataShared = boost::shared_ptr<SmartMet::Engine::Querydata::QImpl>;

  NetcdfResponse() : mEngine(NULL), mDebugLevel(0) {}
  virtual ~NetcdfResponse() {}
  void setDebugLevel(const DebugLevelType& debugLevel) { mDebugLevel = debugLevel; }
  void setEngine(const SmartMet::Spine::SmartMetEngine* engine);
  void setParamConfig(const ParamConfig::Shared paramConfig);
  virtual void get(std::ostream& output);

 protected:
  inline const Spine::SmartMetEngine* getEngine() const { return mEngine; }
  const ParamConfig::Shared getParamConfig() const { return mParamConfig; }
  const DebugLevelType& getDebugLevel() const { return mDebugLevel; }
  IdValueZPair solveNearestLevel(QdataShared q, const ValueZ& levelValue);
  IdValueTPair solveNearestTime(QdataShared q, const ValueT& timeValue);
  IdValueXPair solveNearestX(QdataShared q, const ValueX& xValue);
  IdValueYPair solveNearestY(QdataShared q, const ValueY& yValue);
  RangeZ solveRangeZ(QdataShared q);
  RangeT solveRangeT(QdataShared q);

 private:
  const SmartMet::Spine::SmartMetEngine* mEngine;
  ParamConfig::Shared mParamConfig;
  DebugLevelType mDebugLevel;
};
}  // namespace WCS
}  // namespace Plugin
}  // namespace SmartMet
