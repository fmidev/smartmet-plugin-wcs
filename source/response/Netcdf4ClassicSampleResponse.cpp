#include "Netcdf4ClassicSampleResponse.h"
#include "UniqueTemporaryPath.h"

namespace SmartMet
{
namespace Plugin
{
namespace WCS
{
void Netcdf4ClassicSampleResponse::get(std::ostream& output)
{
  if (not getEngine())
  {
    throw std::runtime_error("Engine is not set.");
  }

  if (not getOptions())
  {
    throw std::runtime_error("Response query options is not set.");
  }

  // We are writing 2D data, a 6 x 12 grid.

  static const int NTime = 1;
  static const int NLevel = 1;
  static const int NX = 6;
  static const int NY = 12;
  float x[NX];
  float y[NY];
  float dataOut[NX][NY];

  int times[NTime] = {0};
  int levels[NLevel] = {0};

  for (int i = 0; i < NX; i++)
    x[i] = 25.0 + 0.1 * i;

  for (int j = 0; j < NY; j++)
    y[j] = 60.0 + 0.1 * j;

  for (int i = 0; i < NX; i++)
    for (int j = 0; j < NY; j++)
      dataOut[i][j] = 0.1 * i * NY + j;

  UniqueTemporaryPath::Unique name1(new UniqueTemporaryPath());
  NcFile dataFile(name1->c_str(), NcFile::FileMode::Replace, NULL, 0, NcFile::Netcdf4Classic);

  dataFile.add_att("Conventions", "CF-1.6");
  dataFile.add_att("title", "Dummy data file");
  dataFile.add_att("institution", "fmi.fi");
  dataFile.add_att("source", "Dummy generator");

  const NcType ncType = ncFloat;
  NcVar* var = NULL;

  var = dataFile.add_var("crs", ncShort);
  var->add_att("grid_mapping_name", "latitude_longitude");
  var->add_att("longitude_of_prime_meridian", 0.0);
  var->add_att("semi_major_axis", 6378137.0);
  var->add_att("inverse_flattening", 298.257223563);

  const NcDim* timeDim = dataFile.add_dim("time", NTime);
  var = dataFile.add_var("atime", ncInt, timeDim);
  var->add_att("long_name", "time");
  var->add_att("calendar", "gregorian");
  var->add_att("units", "minutes since 2016-05-18T09:00:00Z");
  var->put(times, NTime);

  const NcDim* yDim = dataFile.add_dim("lat", NY);
  var = dataFile.add_var("lat", ncType, yDim);
  var->add_att("standard_name", "latitude");
  var->add_att("units", "degrees_north");
  var->add_att("axis", "Y");
  var->add_att("long_name", "latitude");
  var->put(y, NY);

  const NcDim* xDim = dataFile.add_dim("lon", NX);
  var = dataFile.add_var("lon", ncType, xDim);
  var->add_att("standard_name", "longitude");
  var->add_att("units", "degrees_east");
  var->add_att("axis", "X");
  var->add_att("long_name", "longitude");
  var->put(x, NX);

  const NcDim* levelDim = dataFile.add_dim("level", NLevel);
  var = dataFile.add_var("alevel", ncType, levelDim);
  var->add_att("standard_name", "level");
  var->add_att("units", "m");
  var->add_att("axis", "Z");
  var->add_att("long_name", "height");
  var->add_att("positive", "up");
  var->put(levels, NLevel);

  var = dataFile.add_var("data", ncType, timeDim, levelDim, xDim, yDim);
  var->add_att("_FillValue", 32700.f);
  var->add_att("missing_value", 32700.f);
  var->add_att("grid_mapping", "crs");
  var->put(*dataOut, NTime, NLevel, NX, NY);

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
