#include "NetcdfResponse.h"
#include "Options.h"
#include "WcsConvenience.h"
#include "WcsException.h"
#include <exception>
#include <fstream>

namespace SmartMet
{
namespace Plugin
{
namespace WCS
{
void NetcdfResponse::get(std::ostream& output)
{
}

void NetcdfResponse::setEngine(const Spine::SmartMetEngine* engine)
{
  if (engine == NULL)
    throw std::runtime_error("No Engine available");

  if (const Engine::Querydata::Engine* e = dynamic_cast<const Engine::Querydata::Engine*>(engine))
  {
    mEngine = e;
  }
  else
  {
    std::ostringstream msg;
    msg << "Unsupported class object '" << typeid(engine).name()
        << "' passed into 'NetcdfResponse::setEngine'";
    throw std::runtime_error(msg.str());
  }
}

void NetcdfResponse::setParamConfig(const std::shared_ptr<ParamConfig> paramConfig)
{
  mParamConfig = std::dynamic_pointer_cast<NetcdfParamConfig>(paramConfig);
  if (not mParamConfig)
  {
    std::ostringstream msg;
    msg << "Dynamic pointer cast from ParamConfig to NetcdfParamConfig failed";
    throw std::runtime_error(msg.str());
  }
}

std::pair<unsigned long, float> NetcdfResponse::solveNearestLevel(
    boost::shared_ptr<Engine::Querydata::QImpl> q, const float& levelValue)
{
  q->firstLevel();
  unsigned long nearestLevelIndex = q->levelIndex();
  float nearestLevelValue = q->levelValue();
  float distanceToNearestLevel = std::fabs(nearestLevelValue - levelValue);
  q->resetLevel();
  while (q->nextLevel())
  {
    float distanceCandidate = std::fabs(q->levelValue() - levelValue);
    if (distanceCandidate < distanceToNearestLevel)
    {
      distanceToNearestLevel = distanceCandidate;
      nearestLevelIndex = q->levelIndex();
      nearestLevelValue = q->levelValue();
    }

    if (distanceToNearestLevel == 0.0)
      break;
  }

  return std::make_pair(nearestLevelIndex, nearestLevelValue);
}

std::pair<unsigned long, boost::posix_time::ptime> NetcdfResponse::solveNearestTime(
    boost::shared_ptr<Engine::Querydata::QImpl> q, const boost::posix_time::ptime& timeValue)
{
  q->firstTime();
  auto nearestTimeIndex = q->timeIndex();
  auto nearestTimeValue = q->validTime();
  boost::posix_time::ptime t = q->validTime();
  unsigned long distanceToNearestTime =
      std::abs(boost::posix_time::time_duration(t - timeValue).total_seconds());
  q->resetTime();
  while (q->nextTime())
  {
    t = q->validTime();
    unsigned long distanceCandidate =
        std::abs(boost::posix_time::time_duration(t - timeValue).total_seconds());
    if (distanceCandidate < distanceToNearestTime)
    {
      distanceToNearestTime = distanceCandidate;
      nearestTimeIndex = q->timeIndex();
      nearestTimeValue = q->validTime();
    }

    if (distanceToNearestTime == 0)
      break;
  }

  return std::make_pair(nearestTimeIndex, nearestTimeValue);
}

std::pair<unsigned long, double> NetcdfResponse::solveNearestX(
    boost::shared_ptr<Engine::Querydata::QImpl> q, const double& xValue)
{
  if (not q->isGrid())
    throw WcsException(WcsException::NO_APPLICABLE_CODE, "Missing Grid.");

  std::shared_ptr<Options> opt = std::dynamic_pointer_cast<Options>(ResponseBase::getOptions());
  if (not opt)
    throw std::runtime_error("Cannot cast query options object from OptionsBase to Options.");

  auto transformation = opt->getTransformation();
  if (not transformation)
    throw std::runtime_error("Transformation missing!");

  bool swapCoord = opt->getSwap();

  q->firstTime();
  q->firstLevel();

  auto xNumber = q->grid().XNumber();
  auto yNumber = q->grid().YNumber();

  // Choose a line in the midle of grid (y-direction).
  unsigned long startId = yNumber / 2 * xNumber;
  unsigned long id = startId;

  auto nearestLocIndex = startId;
  auto nearestLocValue = q->latLon(startId);
  NFmiPoint tLoc = swap(transformation->transform(swap(nearestLocValue, true)), swapCoord);
  double distanceToNearestLoc = std::abs(tLoc.X() - xValue);

  while (id < startId + xNumber)
  {
    tLoc = swap(transformation->transform(swap(q->latLon(id), true)), swapCoord);
    double distanceCandidate = std::abs(tLoc.X() - xValue);
    if (distanceCandidate < distanceToNearestLoc)
    {
      distanceToNearestLoc = distanceCandidate;
      nearestLocIndex = id;
      nearestLocValue = tLoc;
    }

    if (distanceToNearestLoc < 0.000001)
      break;

    id++;
  }

  return std::make_pair(nearestLocIndex - startId, nearestLocValue.X());
}

std::pair<unsigned long, double> NetcdfResponse::solveNearestY(
    boost::shared_ptr<Engine::Querydata::QImpl> q, const double& yValue)
{
  if (not q->isGrid())
    throw WcsException(WcsException::NO_APPLICABLE_CODE, "Missing Grid.");

  std::shared_ptr<Options> opt = std::dynamic_pointer_cast<Options>(ResponseBase::getOptions());
  if (not opt)
    throw std::runtime_error("Cannot cast query options object from OptionsBase to Options.");

  auto transformation = opt->getTransformation();
  if (not transformation)
    throw std::runtime_error("Transformation missing!");

  bool swapCoord = opt->getSwap();

  q->firstTime();
  q->firstLevel();
  auto xNumber = q->grid().XNumber();
  auto yNumber = q->grid().YNumber();
  auto maxNumber = xNumber * yNumber;

  unsigned long startId = xNumber / 2;
  unsigned long id = startId;

  auto nearestLocIndex = startId;
  auto nearestLocValue = q->latLon(startId);
  NFmiPoint tLoc = swap(transformation->transform(swap(nearestLocValue, true)), swapCoord);
  double distanceToNearestLoc = std::abs(tLoc.Y() - yValue);
  while (id < maxNumber)
  {
    tLoc = swap(transformation->transform(swap(q->latLon(id), true)), swapCoord);
    double distanceCandidate = std::abs(tLoc.Y() - yValue);
    if (distanceCandidate < distanceToNearestLoc)
    {
      distanceToNearestLoc = distanceCandidate;
      nearestLocIndex = id;
      nearestLocValue = tLoc;
    }

    if (distanceToNearestLoc < 0.000001)
      break;

    id += xNumber;
  }

  return std::make_pair((nearestLocIndex / xNumber), nearestLocValue.Y());
}
}
}
}
