#include "Netcdf4ClassicResponse.h"
#include "DataContainer.h"
#include "Options.h"
#include "ParamConfig.h"
#include "UniqueTemporaryPath.h"
#include "WcsConvenience.h"
#include "WcsException.h"
#include <macgyver/StringConversion.h>
#include <spine/ParameterFactory.h>

namespace SmartMet
{
namespace Plugin
{
namespace WCS
{
void Netcdf4ClassicResponse::get(std::ostream& output)
{
  const auto querydata = dynamic_cast<const Engine::Querydata::Engine*>(getEngine());
  if (not querydata)
    throw std::runtime_error("Cast from SmartMetEngine to Querydata failded.");

  if (not getOptions())
    throw std::runtime_error("Options object is not set.");

  auto opt = std::dynamic_pointer_cast<Options>(getOptions());
  if (not opt)
    throw std::runtime_error("Cannot cast query options object from OptionsBase to Options.");

  auto q = querydata->get(opt->getProducer());
  if (not q)
  {
    std::ostringstream msg;
    msg << "Data not found: '" << opt->getProducer() + "'.";
    throw WcsException(WcsException::NO_APPLICABLE_CODE, msg.str());
  }

  if (not q->firstLevel())
  {
    std::ostringstream msg;
    msg << "Level data not found: '" << opt->getProducer() + "'.";
    throw WcsException(WcsException::NO_APPLICABLE_CODE, msg.str());
  }

  if (not q->firstTime())
  {
    std::ostringstream msg;
    msg << "Time dimension not found: '" << opt->getProducer() + "'.";
    throw WcsException(WcsException::NO_APPLICABLE_CODE, msg.str());
  }

  if (not q->isGrid())
    throw WcsException(WcsException::NO_APPLICABLE_CODE, "Missing Grid.");

  boost::optional<Spine::Parameter> parameter =
      Spine::ParameterFactory::instance().parse(opt->getParameterName());
  if (not parameter)
  {
    std::ostringstream msg;
    msg << "ParameterFactory can not parse '" << opt->getParameterName() << "' parameter of '"
        << opt->getProducer() << "' producer.";
    throw WcsException(WcsException::NO_APPLICABLE_CODE, msg.str());
  }

  if (not q->param(parameter->number()))
  {
    std::ostringstream msg;
    msg << "Producer '" << opt->getProducer() << "' data not found for '" << opt->getParameterName()
        << "' parameter\n";
    throw WcsException(WcsException::NO_APPLICABLE_CODE, msg.str());
  }

  auto rangeZ = solveRangeZ(q);
  q->firstLevel();

  std::vector<unsigned long> levelIds;
  boost::optional<DimensionTrim> trim;
  boost::optional<DimensionSlice> slice;

  if (slice = opt->getDimensionSlice("z"))
  {
    const double sPoint = slice->get("SlicePoint").get_double();
    if (sPoint < rangeZ.first or sPoint > rangeZ.second)
    {
      std::ostringstream msg;
      msg << "SlicePoint '" << sPoint << "' is out of z range [" << rangeZ.first << ","
          << rangeZ.second << "]";
      WcsException wcsException(WcsException::INVALID_PARAMETER_VALUE, msg.str());
      wcsException.setLocation("DimensionSlice");
      throw wcsException;
    }
    const auto nearestLevel = solveNearestLevel(q, sPoint);
    levelIds.push_back(nearestLevel.first);
  }
  else if (trim = opt->getDimensionTrim("z"))
  {
    const auto low = trim->get("TrimLow").get_double();
    const auto high = trim->get("TrimHigh").get_double();
    if (high < rangeZ.first or low > rangeZ.second)
    {
      std::ostringstream msg;
      msg << "DimensionTrim range '[" << low << "," << high << "]' is out of z range ["
          << rangeZ.first << "," << rangeZ.second << "]";
      WcsException wcsException(WcsException::INVALID_PARAMETER_VALUE, msg.str());
      wcsException.setLocation("DimensionTrim");
      throw wcsException;
    }
    auto nearestLow = solveNearestLevel(q, low);
    auto nearesthigh = solveNearestLevel(q, high);
    for (auto id = nearestLow.first; id <= nearesthigh.first; ++id)
      levelIds.push_back(id);
  }
  else
  {
    levelIds.push_back(q->levelIndex());
  }

  float missingValue = kFloatMissing;
  const unsigned long NX = q->grid().XNumber();
  const unsigned long NY = q->grid().YNumber();
  unsigned long minXId = 0;
  unsigned long minYId = 0;
  unsigned long maxXId = NX - 1;
  unsigned long maxYId = NY - 1;
  unsigned long xDelta = NX;
  unsigned long yDelta = NY;
  unsigned long zDelta = levelIds.size();

  const auto& qWgs84Envelope = q->getWGS84Envelope();
  const auto& rangeLon = qWgs84Envelope.getRangeLon();

  if (slice = opt->getDimensionSlice("x"))
  {
    const auto step = (rangeLon.getMax() - rangeLon.getMin()) / maxXId;
    const Engine::Querydata::Range range(rangeLon.getMin() - step, rangeLon.getMax() + step);
    const auto sPoint = slice->get("SlicePoint").get_double();
    if (sPoint < range.getMin() or sPoint > range.getMax())
    {
      std::ostringstream msg;
      msg << "SlicePoint '" << sPoint << "' is out of x range [" << range.getMin() << ","
          << range.getMax() << "]";
      WcsException wcsException(WcsException::INVALID_PARAMETER_VALUE, msg.str());
      wcsException.setLocation("DimensionSlice");
      throw wcsException;
    }

    auto nearestLoc = solveNearestX(q, sPoint);
    minXId = nearestLoc.first;
    maxXId = nearestLoc.first;
    xDelta = maxXId - minXId + 1;
  }
  else if (trim = opt->getDimensionTrim("x"))
  {
    double low = trim->get("TrimLow").get_double();
    double high = trim->get("TrimHigh").get_double();
    if (high < low)
    {
      double tmp = low;
      low = high;
      high = tmp;
    }

    if (high < rangeLon.getMin() or low > rangeLon.getMax())
    {
      std::ostringstream msg;
      msg << "DimensionTrim range '[" << low << "," << high << "]' is out of x range ["
          << rangeLon.getMin() << "," << rangeLon.getMax() << "]";
      WcsException wcsException(WcsException::INVALID_PARAMETER_VALUE, msg.str());
      wcsException.setLocation("DimensionTrim");
      throw wcsException;
    }

    auto nearestLow = solveNearestX(q, low);
    auto nearestHigh = solveNearestX(q, high);
    minXId = nearestLow.first;
    maxXId = nearestHigh.first;
    xDelta = maxXId - minXId + 1;
  }

  const auto& rangeLat = qWgs84Envelope.getRangeLat();

  if (slice = opt->getDimensionSlice("y"))
  {
    const auto step = (rangeLat.getMax() - rangeLat.getMin()) / maxYId;
    const Engine::Querydata::Range range(rangeLat.getMin() - step, rangeLat.getMax() + step);
    const auto sPoint = slice->get("SlicePoint").get_double();
    if (sPoint < range.getMin() or sPoint > range.getMax())
    {
      std::ostringstream msg;
      msg << "SlicePoint '" << sPoint << "' is out of y range [" << range.getMin() << ","
          << range.getMax() << "]";
      WcsException wcsException(WcsException::INVALID_PARAMETER_VALUE, msg.str());
      wcsException.setLocation("DimensionSlice");
      throw wcsException;
    }

    auto nearestLoc = solveNearestY(q, sPoint);
    minYId = nearestLoc.first;
    maxYId = nearestLoc.first;
    yDelta = maxYId - minYId + 1;
  }
  else if (trim = opt->getDimensionTrim("y"))
  {
    double low = trim->get("TrimLow").get_double();
    double high = trim->get("TrimHigh").get_double();
    if (high < low)
    {
      double tmp = low;
      low = high;
      high = tmp;
    }

    if (high < rangeLat.getMin() or low > rangeLat.getMax())
    {
      std::ostringstream msg;
      msg << "DimensionTrim range '[" << low << "," << high << "]' is out of y range ["
          << rangeLat.getMin() << "," << rangeLat.getMax() << "]";
      WcsException wcsException(WcsException::INVALID_PARAMETER_VALUE, msg.str());
      wcsException.setLocation("DimensionTrim");
      throw wcsException;
    }
    auto nearestLow = solveNearestY(q, low);
    auto nearestHigh = solveNearestY(q, high);
    minYId = nearestLow.first;
    maxYId = nearestHigh.first;
    yDelta = maxYId - minYId + 1;
  }

  std::pair<boost::posix_time::ptime, boost::posix_time::ptime> rangeT = solveRangeT(q);

  std::vector<long> t;
  std::vector<unsigned long> timeIds;
  if (slice = opt->getDimensionSlice("t"))
  {
    const auto sPoint = slice->get("SlicePoint").get_ptime();
    if (sPoint < rangeT.first or sPoint > rangeT.second)
    {
      std::ostringstream msg;
      msg << "SlicePoint '" << sPoint << "' is out of t range [" << rangeT.first << ","
          << rangeT.second << "]";
      WcsException wcsException(WcsException::INVALID_PARAMETER_VALUE, msg.str());
      wcsException.setLocation("DimensionSlice");
      throw wcsException;
    }

    // Return only the nearest time slice
    auto nearestTime = solveNearestTime(q, sPoint);
    q->firstLevel();
    q->resetTime();
    if (not q->timeIndex(nearestTime.first))
      throw WcsException(WcsException::NO_APPLICABLE_CODE, "Cannot set a time dimension slice");

    timeIds.push_back(nearestTime.first);
    t.push_back(q->validTime().DifferenceInMinutes(q->originTime()));
  }
  else if (trim = opt->getDimensionTrim("t"))
  {
    auto low = trim->get("TrimLow").get_ptime();
    auto high = trim->get("TrimHigh").get_ptime();
    if (high < rangeT.first or low > rangeT.second)
    {
      std::ostringstream msg;
      msg << "DimensionTrim range '[" << low << "," << high << "]' is out of t range ["
          << rangeT.first << "," << rangeT.second << "]";
      WcsException wcsException(WcsException::INVALID_PARAMETER_VALUE, msg.str());
      wcsException.setLocation("DimensionTrim");
      throw wcsException;
    }

    auto nearestLow = solveNearestTime(q, low);
    auto nearestHigh = solveNearestTime(q, high);
    q->firstLevel();
    q->resetTime();
    for (unsigned long id = nearestLow.first; id <= nearestHigh.first; id++)
    {
      if (not q->timeIndex(id))
        throw WcsException(WcsException::NO_APPLICABLE_CODE, "Cannot set a time dimension: trim");

      timeIds.push_back(q->timeIndex());

      t.push_back(q->validTime().DifferenceInMinutes(q->originTime()));
    }
  }
  else
  {
    // Return all the time steps
    q->firstLevel();
    q->resetTime();
    while (q->nextTime())
    {
      timeIds.push_back(q->timeIndex());
      t.push_back(q->validTime().DifferenceInMinutes(q->originTime()));
    }
  }

  if (getDebugLevel() > 2)
  {
    std::cout << "DataTime: " << q->validTime() << "\nOriginTime: " << q->originTime()
              << "\nProducer: " << opt->getProducer() << "\nParameterId: " << q->parameterName()
              << "\nParameterName: " << parameter->name() << "\nLevelName: " << q->levelName()
              << "\nLevelValue: " << q->levelValue() << "\nGridSizeX: " << q->grid().XNumber()
              << "\nGridSizeY: " << q->grid().YNumber() << "\n";
  }

  unsigned long tDelta = t.size();

  DataContainer<float> xc(xDelta * yDelta, "X coordinates");
  DataContainer<float> yc(xDelta * yDelta, "Y coordinates");
  DataContainer<float> zc(zDelta, "Z coordinates");
  DataContainer<float> dataOut(xDelta * yDelta * zDelta * tDelta, "Data values");

  auto transformation = opt->getTransformation();

  // XY coordinates
  bool swapCoord = opt->getSwap();
  auto llCache = q->latLonCache();
  auto it = llCache->cbegin();
  auto itEnd = it;
  for (unsigned long yId = minYId; yId <= maxYId; yId++)
  {
    it = llCache->cbegin() + yId * NX + minXId;
    itEnd = it + xDelta;
    while (it < itEnd)
    {
      NFmiPoint tP = swap(transformation->transform(swap(*it++, true)), swapCoord);
      xc.add(tP.X());
      yc.add(tP.Y());
    }
  }
  for (const auto& lId : levelIds)
  {
    if (not q->levelIndex(lId))
      throw std::runtime_error("Cannot set a data level to get ");
    zc.add(q->levelValue());
  }

  NFmiDataMatrix<float> dem;
  NFmiDataMatrix<bool> flags;

  q->resetLevel();

  ParamConfig::ParamIdType parameterId = parameter->number();
  std::shared_ptr<ParamConfig::ParamMetaItemType> paramMetaItem;
  auto paramConfig = getParamConfig();
  if (paramConfig)
    paramMetaItem = paramConfig->getParamMetaItem(parameterId);

  if (not paramMetaItem)
  {
    std::ostringstream msg;
    msg << "Parameter '" << parameterId << "' meta data configuration is missing";
    throw WcsException(WcsException::NO_APPLICABLE_CODE, msg.str());
  }

  for (const auto& tId : timeIds)
  {
    if (not q->timeIndex(tId))
    {
      std::ostringstream msg;
      msg << "Error occured while setting time index: " << tId;
      throw std::runtime_error(msg.str());
    }

    for (const auto& lId : levelIds)
    {
      q->levelIndex(lId);

      NFmiDataMatrix<float> dataMatrix;
      q->croppedValues(dataMatrix, minXId, minYId, maxXId, maxYId, dem, flags);

      if (dataMatrix.NX() != xDelta or dataMatrix.NY() != yDelta)
        throw WcsException(WcsException::NO_APPLICABLE_CODE, "Data size error.");

      for (unsigned long yId = 0; yId < yDelta; yId++)
      {
        for (unsigned xId = 0; xId < xDelta; xId++)
        {
          float val = dataMatrix.At(xId, yId, missingValue);
          if (val == missingValue)
            dataOut.add(val);
          else
            dataOut.add(paramMetaItem->mConversionScale * val + paramMetaItem->mConversionBase);
        }
      }
    }
  }

  UniqueTemporaryPath::Unique name1(new UniqueTemporaryPath());
  NcFile dataFile(name1->get().c_str(), NcFile::FileMode::Replace, NULL, 0, NcFile::Netcdf4Classic);
  if (not dataFile.is_valid())
  {
    std::ostringstream msg;
    msg << "Cannot create or open temporary path passed to NcFile object: '" << name1->get().c_str()
        << "'.";
    throw WcsException(WcsException::NO_APPLICABLE_CODE, msg.str());
  }

  dataFile.add_att("Conventions", "CF-1.6");
  dataFile.add_att(
      "title", std::string("Dataset from ").append(opt->getProducer()).append(" producer").c_str());
  dataFile.add_att("institution", "fmi.fi");
  dataFile.add_att("source", "<undefined>");
  dataFile.add_att("references", "<undefined>");
  dataFile.add_att("comment", "<undefined>");

  const NcType ncType = ncFloat;
  NcVar* var = NULL;
  const std::string grid_mapping_name = "latitude_longitude";
  if (grid_mapping_name == "latitude_longitude")
  {
    var = dataFile.add_var("grid_mapping", ncShort);
    var->add_att("grid_mapping_name", "latitude_longitude");
  }
  else if (grid_mapping_name == "rotated_latitude_longitude")
  {
    var = dataFile.add_var("grid_mapping", ncChar);
    var->add_att("grid_mapping_name", "rotated_latitude_longitude");
    var->add_att("grid_north_pole_latitude", 30.0);
    var->add_att("grid_north_pole_longitude", 0.0);
  }
  else
  {
    throw std::runtime_error("NetCDF grid_mapping is not configured");
  }

  const NcDim* timeDim = dataFile.add_dim("time", tDelta);
  var = dataFile.add_var("time", ncInt, timeDim);
  var->add_att("long_name", "time");
  var->add_att("calendar", "gregorian");
  var->add_att("units",
               std::string("minutes since ")
                   .append(Fmi::to_iso_string(q->originTime().PosixTime()))
                   .append("Z")
                   .c_str());
  var->put(t.data(), tDelta);

  const NcDim* levelDim = dataFile.add_dim("level", zDelta);
  var = dataFile.add_var("level", ncType, levelDim);
  var->add_att("standard_name", "level");
  var->add_att("units", "m");
  var->add_att("axis", "Z");
  var->add_att("long_name", "height");
  var->add_att("positive", "up");
  var->put(zc.array().get(), zDelta);

  const NcDim* ycDim = dataFile.add_dim("yc", yDelta);
  const NcDim* xcDim = dataFile.add_dim("xc", xDelta);

  var = dataFile.add_var("lat", ncType, ycDim, xcDim);
  var->add_att("standard_name", "latitude");
  var->add_att("units", "degrees_north");
  var->add_att("axis", "Y");
  var->add_att("long_name", "latitude");
  var->put(yc.array().get(), yDelta, xDelta);

  var = dataFile.add_var("lon", ncType, ycDim, xcDim);
  var->add_att("standard_name", "longitude");
  var->add_att("units", "degrees_east");
  var->add_att("axis", "X");
  var->add_att("long_name", "longitude");
  var->put(xc.array().get(), yDelta, xDelta);

  std::string varName = Fmi::ascii_tolower_copy(parameter->name());
  varName.append("_").append(Fmi::to_string(parameterId));
  var = dataFile.add_var(varName.c_str(), ncType, timeDim, levelDim, ycDim, xcDim);
  var->add_att("_FillValue", missingValue);
  var->add_att("missing_value", missingValue);
  var->add_att("grid_mapping", "grid_mapping");
  var->add_att("coordinates", "lon lat");

  if (paramMetaItem->mStdName)
    var->add_att("standard_name", paramMetaItem->mStdName.get().c_str());
  if (paramMetaItem->mUnit)
    var->add_att("units", paramMetaItem->mUnit.get().c_str());
  if (paramMetaItem->mLongName)
    var->add_att("long_name", paramMetaItem->mLongName.get().c_str());

  var->put(dataOut.array().get(), tDelta, zDelta, yDelta, xDelta);

  dataFile.close();

  std::ifstream ifs;
  ifs.open(name1->get().c_str(), std::ifstream::in);
  if (ifs.is_open())
  {
    output << ifs.rdbuf();
    ifs.close();
  }
  else
    output << "";

  // Delete file
  std::remove(name1->get().c_str());
}
}
}
}
