#pragma once

#include <boost/shared_ptr.hpp>
#include <string>

namespace SmartMet
{
namespace Plugin
{
namespace WCS
{
template <typename T = float>
class DataContainer
{
 public:
  using Name = std::string;
  using Size = unsigned long;
  using DataType = T;
  using SharedArray = boost::shared_ptr<DataType[]>;
  explicit DataContainer(const Size& size, Name name);

  void add(const DataType& value);
  SharedArray array() const;
  Size size() const;

 private:
  DataContainer& operator=(const DataContainer& other) = delete;
  DataContainer(const DataContainer& other) = delete;

  Name mName;
  Size mCounter;
  Size mSize;
  SharedArray mData;
};
}
}
}
