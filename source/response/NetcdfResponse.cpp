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
void NetcdfResponse::get(std::ostream& output) {}

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

void NetcdfResponse::setParamConfig(const ParamConfig::Shared paramConfig)
{
  mParamConfig = std::dynamic_pointer_cast<NetcdfParamConfig>(paramConfig);
  if (not mParamConfig)
  {
    std::ostringstream msg;
    msg << "Dynamic pointer cast from ParamConfig to NetcdfParamConfig failed";
    throw std::runtime_error(msg.str());
  }
}

NetcdfResponse::IdValueZPair NetcdfResponse::solveNearestLevel(QdataShared q,
                                                               const ValueZ& levelValue)
{
  q->firstLevel();
  Id nearestLevelIndex = q->levelIndex();
  ValueZ nearestLevelValue = q->levelValue();
  ValueZ distanceToNearestLevel = std::fabs(nearestLevelValue - levelValue);
  q->resetLevel();
  while (q->nextLevel())
  {
    ValueZ distanceCandidate = std::fabs(q->levelValue() - levelValue);
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

NetcdfResponse::IdValueTPair NetcdfResponse::solveNearestTime(QdataShared q,
                                                              const ValueT& timeValue)
{
  q->firstTime();
  Id nearestTimeIndex = q->timeIndex();
  ValueT nearestTimeValue = q->validTime();
  ValueT t = q->validTime();
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

NetcdfResponse::IdValueXPair NetcdfResponse::solveNearestX(QdataShared q, const ValueX& xValue)
{
  if (not q->isGrid())
    throw WcsException(WcsException::NO_APPLICABLE_CODE, "Missing Grid.");

  Options::Shared opt = std::dynamic_pointer_cast<Options>(ResponseBase::getOptions());
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
  Id startId = yNumber / 2 * xNumber;
  Id id = startId;

  auto nearestLocIndex = startId;
  auto nearestLocValue = q->latLon(startId);
  NFmiPoint tLoc = swap(transformation->transform(swap(nearestLocValue, true)), swapCoord);
  ValueX distanceToNearestLoc = std::abs(tLoc.X() - xValue);

  while (id < startId + xNumber)
  {
    tLoc = swap(transformation->transform(swap(q->latLon(id), true)), swapCoord);
    ValueX distanceCandidate = std::abs(tLoc.X() - xValue);
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

NetcdfResponse::IdValueYPair NetcdfResponse::solveNearestY(QdataShared q, const ValueY& yValue)
{
  if (not q->isGrid())
    throw WcsException(WcsException::NO_APPLICABLE_CODE, "Missing Grid.");

  Options::Shared opt = std::dynamic_pointer_cast<Options>(ResponseBase::getOptions());
  if (not opt)
    throw std::runtime_error("Cannot cast query options object from OptionsBase to Options.");

  auto transformation = opt->getTransformation();
  if (not transformation)
    throw std::runtime_error("Transformation missing!");

  bool swapCoord = opt->getSwap();

  q->firstTime();
  q->firstLevel();
  Id xNumber = q->grid().XNumber();
  Id yNumber = q->grid().YNumber();
  Id maxNumber = xNumber * yNumber;

  Id startId = xNumber / 2;
  Id id = startId;

  Id nearestLocIndex = startId;
  NFmiPoint nearestLocValue = q->latLon(startId);
  NFmiPoint tLoc = swap(transformation->transform(swap(nearestLocValue, true)), swapCoord);
  ValueY distanceToNearestLoc = std::abs(tLoc.Y() - yValue);
  while (id < maxNumber)
  {
    tLoc = swap(transformation->transform(swap(q->latLon(id), true)), swapCoord);
    ValueY distanceCandidate = std::abs(tLoc.Y() - yValue);
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

NetcdfResponse::RangeZ NetcdfResponse::solveRangeZ(QdataShared q)
{
  q->firstLevel();
  ValueZ low = q->levelValue();
  ValueZ high = low;
  while (q->nextLevel())
    high = q->levelValue();
  if (low > high)
    std::swap(low, high);

  return std::make_pair(low, high);
}

NetcdfResponse::RangeT NetcdfResponse::solveRangeT(QdataShared q)
{
  q->firstTime();
  ValueT low = q->validTime();
  ValueT high = q->validTime();
  q->resetTime();
  while (q->nextTime())
  {
    high = q->validTime();
  }

  return std::make_pair(low, high);
}
}  // namespace WCS
}  // namespace Plugin
}  // namespace SmartMet
