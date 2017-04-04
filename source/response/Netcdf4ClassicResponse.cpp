#include "Netcdf4ClassicResponse.h"
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
  const Engine::Querydata::Engine* querydata =
      dynamic_cast<const Engine::Querydata::Engine*>(getEngine());
  if (not querydata)
    throw std::runtime_error("Cast from SmartMetEngine to Querydata failded.");

  if (not getOptions())
    throw std::runtime_error("Options object is not set.");

  std::shared_ptr<Options> opt = std::dynamic_pointer_cast<Options>(getOptions());
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

  std::vector<unsigned long> levelIds;
  boost::optional<DimensionTrim> trim;
  boost::optional<DimensionSlice> slice;

  if (slice = opt->getDimensionSlice("z"))
  {
    auto nearestLevel = solveNearestLevel(q, slice->get("SlicePoint").get_double());
    levelIds.push_back(nearestLevel.first);
  }
  else if (trim = opt->getDimensionTrim("z"))
  {
    auto nearestLow = solveNearestLevel(q, trim->get("TrimLow").get_double());
    auto nearesthigh = solveNearestLevel(q, trim->get("TrimHigh").get_double());
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

  if (slice = opt->getDimensionSlice("x"))
  {
    auto nearestLoc = solveNearestX(q, slice->get("SlicePoint").get_double());
    minXId = nearestLoc.first;
    maxXId = nearestLoc.first;
    xDelta = maxXId - minXId + 1;

    if (getDebugLevel() > 2)
    {
      std::ostringstream msg;
      msg << "\nDimensionSlice 'x' axis."
          << " GridID=" << nearestLoc.first << " Value=" << nearestLoc.second << "\n";
      std::cout << msg.str() << "\n";
    }
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
    auto nearestLow = solveNearestX(q, low);
    auto nearestHigh = solveNearestX(q, high);
    minXId = nearestLow.first;
    maxXId = nearestHigh.first;
    xDelta = maxXId - minXId + 1;

    if (getDebugLevel() > 2)
    {
      std::ostringstream msg;
      msg << "\nDimensionTrim 'x' axis."
          << " GridID=" << nearestLow.first << " Value=" << nearestLow.second
          << " GridID=" << nearestHigh.first << " Value=" << nearestHigh.second << "\n";
      std::cout << msg.str() << "\n";
    }
  }

  if (slice = opt->getDimensionSlice("y"))
  {
    auto nearestLoc = solveNearestY(q, slice->get("SlicePoint").get_double());
    minYId = nearestLoc.first;
    maxYId = nearestLoc.first;
    yDelta = maxYId - minYId + 1;
    if (getDebugLevel() > 2)
    {
      std::ostringstream msg;
      msg << "DimensionSlice 'y' axis."
          << " GridID=" << nearestLoc.first << " Value=" << nearestLoc.second << "\n";
      std::cout << msg.str() << "\n";
    }
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
    auto nearestLow = solveNearestY(q, low);
    auto nearestHigh = solveNearestY(q, high);
    minYId = nearestLow.first;
    maxYId = nearestHigh.first;
    yDelta = maxYId - minYId + 1;

    if (getDebugLevel() > 2)
    {
      std::ostringstream msg;
      msg << "\nDimensionTrim 'y' axis."
          << " GridID=" << nearestLow.first << " Value=" << nearestLow.second
          << " GridID=" << nearestHigh.first << " Value=" << nearestHigh.second << "\n";
      std::cout << msg.str() << "\n";
    }
  }

  std::vector<long> t;
  std::vector<unsigned long> timeIds;
  if (slice = opt->getDimensionSlice("t"))
  {
    // Return only the nearest time slice
    auto nearestTime = solveNearestTime(q, slice->get("SlicePoint").get_ptime());
    q->firstLevel();
    q->resetTime();
    if (not q->timeIndex(nearestTime.first))
      throw WcsException(WcsException::NO_APPLICABLE_CODE, "Cannot set a time dimension slice");

    timeIds.push_back(nearestTime.first);
    t.push_back(q->validTime().DifferenceInMinutes(q->originTime()));
  }
  else if (trim = opt->getDimensionTrim("t"))
  {
    auto nearestLow = solveNearestTime(q, trim->get("TrimLow").get_ptime());
    auto nearestHigh = solveNearestTime(q, trim->get("TrimHigh").get_ptime());
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

  boost::shared_ptr<float[]> xc(new float[xDelta * yDelta]);
  boost::shared_ptr<float[]> yc(new float[xDelta * yDelta]);
  boost::shared_ptr<float[]> z(new float[zDelta]);
  boost::shared_ptr<float[]> dataOut(new float[xDelta * yDelta * zDelta * t.size()]);

  auto transformation = opt->getTransformation();

  // XY coordinates
  bool swapCoord = opt->getSwap();
  auto llCache = q->latLonCache();
  unsigned long coordBase = 0;
  auto it = llCache->cbegin();
  auto itEnd = it;
  for (unsigned long yId = minYId; yId <= maxYId; yId++)
  {
    it = llCache->cbegin() + yId * NX + minXId;
    itEnd = it + xDelta;
    while (it < itEnd)
    {
      NFmiPoint tP = swap(transformation->transform(swap(*it++, true)), swapCoord);
      xc[coordBase] = tP.X();
      yc[coordBase++] = tP.Y();
    }
  }
  for (unsigned long lId = 0; lId < zDelta; lId++)
  {
    if (not q->levelIndex(levelIds.at(lId)))
      throw std::runtime_error("Cannot set a data level to get ");
    z[lId] = q->levelValue();
  }

  NFmiDataMatrix<float> dem;
  NFmiDataMatrix<bool> flags;
  unsigned long tbase = 0;

  q->resetLevel();
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

  for (unsigned long tId = 0; tId < timeIds.size(); tId++)
  {
    if (not q->timeIndex(timeIds.at(tId)))
    {
      std::ostringstream msg;
      msg << "Error occured while setting time index: " << timeIds.at(tId);
      throw std::runtime_error(msg.str());
    }

    for (unsigned long lId = 0; lId < levelIds.size(); lId++)
    {
      q->levelIndex(levelIds.at(lId));

      NFmiDataMatrix<float> dataMatrix;
      q->values(dataMatrix, dem, flags);

      if (dataMatrix.NX() != NX or dataMatrix.NY() != NY)
        throw WcsException(WcsException::NO_APPLICABLE_CODE, "Data size error.");

      for (unsigned long yId = minYId; yId <= maxYId; yId++)
      {
        for (unsigned xId = minXId; xId <= maxXId; tbase++, xId++)
        {
          float val = dataMatrix.At(xId, yId, missingValue);
          if (val == missingValue)
            dataOut[tbase] = val;
          else
            dataOut[tbase] = paramMetaItem->mConversionScale * val + paramMetaItem->mConversionBase;
        }
      }
    }
  }

  UniqueTemporaryPath::Unique name1(new UniqueTemporaryPath());
  NcFile dataFile(name1->c_str(), NcFile::FileMode::Replace, NULL, 0, NcFile::Netcdf4Classic);
  if (not dataFile.is_valid())
  {
    std::ostringstream msg;
    msg << "Cannot create or open temporary path passed to NcFile object: '" << name1->c_str()
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

  const NcDim* timeDim = dataFile.add_dim("time", t.size());
  var = dataFile.add_var("time", ncInt, timeDim);
  var->add_att("long_name", "time");
  var->add_att("calendar", "gregorian");
  var->add_att("units",
               std::string("minutes since ")
                   .append(Fmi::to_iso_string(q->originTime().PosixTime()))
                   .append("Z")
                   .c_str());
  var->put(t.data(), t.size());

  const NcDim* levelDim = dataFile.add_dim("level", zDelta);
  var = dataFile.add_var("level", ncType, levelDim);
  var->add_att("standard_name", "level");
  var->add_att("units", "m");
  var->add_att("axis", "Z");
  var->add_att("long_name", "height");
  var->add_att("positive", "up");
  var->put(z.get(), zDelta);

  const NcDim* ycDim = dataFile.add_dim("yc", yDelta);
  const NcDim* xcDim = dataFile.add_dim("xc", xDelta);

  var = dataFile.add_var("lat", ncType, ycDim, xcDim);
  var->add_att("standard_name", "latitude");
  var->add_att("units", "degrees_north");
  var->add_att("axis", "Y");
  var->add_att("long_name", "latitude");
  var->put(yc.get(), yDelta, xDelta);

  var = dataFile.add_var("lon", ncType, ycDim, xcDim);
  var->add_att("standard_name", "longitude");
  var->add_att("units", "degrees_east");
  var->add_att("axis", "X");
  var->add_att("long_name", "longitude");
  var->put(xc.get(), yDelta, xDelta);

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

  var->put(dataOut.get(), t.size(), zDelta, yDelta, xDelta);

  dataFile.close();

  std::ifstream ifs;
  ifs.open(name1->c_str(), std::ifstream::in);
  if (ifs.is_open())
  {
    output << ifs.rdbuf();
    ifs.close();
  }
  else
    output << "";

  // Delete file
  std::remove(name1->c_str());
}
}
}
}
