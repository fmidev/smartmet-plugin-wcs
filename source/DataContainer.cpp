#include "DataContainer.h"
#include "WcsException.h"
#include <sstream>

namespace SmartMet
{
namespace Plugin
{
namespace WCS
{
template <typename T>
DataContainer<T>::DataContainer(const Size& size, Name name)
    : mName(name), mCounter(0), mSize(size), mData(new DataType[size])
{
}

template <typename T>
void DataContainer<T>::add(const DataType& value)
{
  if (mCounter < mSize)
  {
    mData[mCounter++] = value;
  }
  else
  {
    std::ostringstream msg;
    msg << "There is too many values to add to '" << mName << "' array. Maximum size is '" << mSize
        << "'";
    throw WcsException(WcsException::NO_APPLICABLE_CODE, msg.str());
  }
}

template <typename T>
typename DataContainer<T>::SharedArray DataContainer<T>::array() const
{
  if (mCounter != mSize)
  {
    std::ostringstream msg;
    msg << "Array '" << mName << "' is not fully fiilled with values. At the moment there is '"
        << mCounter << "/" << mSize << "' values added.";
    throw WcsException(WcsException::NO_APPLICABLE_CODE, msg.str());
  }
  return mData;
}

template <class T>
typename DataContainer<T>::Size DataContainer<T>::size() const
{
  return mSize;
}

// Explicit template instantiation
template class DataContainer<>;
}
}
}
