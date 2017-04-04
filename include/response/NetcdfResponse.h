#pragma once

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

  NetcdfResponse() : mEngine(NULL), mDebugLevel(0) {}
  virtual ~NetcdfResponse() {}
  void setDebugLevel(const DebugLevelType& debugLevel) { mDebugLevel = debugLevel; }
  void setEngine(const SmartMet::Spine::SmartMetEngine* engine);
  void setParamConfig(const std::shared_ptr<ParamConfig> paramConfig);
  virtual void get(std::ostream& output);

 protected:
  inline const Spine::SmartMetEngine* getEngine() const { return mEngine; }
  const std::shared_ptr<ParamConfig> getParamConfig() const { return mParamConfig; }
  const DebugLevelType& getDebugLevel() const { return mDebugLevel; }
  std::pair<unsigned long, float> solveNearestLevel(
      boost::shared_ptr<SmartMet::Engine::Querydata::QImpl> q, const float& levelValue);
  std::pair<unsigned long, boost::posix_time::ptime> solveNearestTime(
      boost::shared_ptr<SmartMet::Engine::Querydata::QImpl> q,
      const boost::posix_time::ptime& timeValue);
  std::pair<unsigned long, double> solveNearestX(
      boost::shared_ptr<SmartMet::Engine::Querydata::QImpl> q, const double& xValue);
  std::pair<unsigned long, double> solveNearestY(
      boost::shared_ptr<SmartMet::Engine::Querydata::QImpl> q, const double& yValue);

 private:
  const SmartMet::Spine::SmartMetEngine* mEngine;
  std::shared_ptr<ParamConfig> mParamConfig;
  DebugLevelType mDebugLevel;
};
}
}
}
