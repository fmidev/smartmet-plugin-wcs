#include "DataSetDef.h"
#include <string>
namespace SmartMet
{
namespace Plugin
{
namespace WCS
{
class AbstractGeometry
{
 protected:
  using IDType = std::string;
  using AnyURIType = std::string;
  using PositiveIntegerType = unsigned;
  using NCNameType = std::string;
  using NCNameListType = std::vector<NCNameType>;

  // AbstractGMLType attributes
  IDType mId;

  // AbstractGeometryType --> SRSReferenceGroup
  AnyURIType mSrsName;
  PositiveIntegerType mSrsDimention;
  NCNameListType mAxisLabels;
  NCNameListType mUomLabels;
};

class CoverageDescription : protected AbstractGeometry
{
 public:
  using CoverageIdType = NCNameType;
  using ProducerType = std::string;
  using ParameterNameType = std::string;

  inline const ProducerType& getProducer() const { return mProducer; }
  inline const ParameterNameType& getParameterName() const { return mParameterName; }
  boost::shared_ptr<DataSetDef> getDataSetDef() const { return mDataSetDef; }
  void setProducer(const ProducerType& producer) { mProducer = producer; };
  void setParameterName(const ParameterNameType& name) { mParameterName = name; }
  void setDataSetDef(const boost::shared_ptr<DataSetDef> dataSetDef) { mDataSetDef = dataSetDef; }

 private:
  ProducerType mProducer;
  ParameterNameType mParameterName;
  boost::shared_ptr<DataSetDef> mDataSetDef;
};
}  // namespace WCS
}  // namespace Plugin
}  // namespace SmartMet
