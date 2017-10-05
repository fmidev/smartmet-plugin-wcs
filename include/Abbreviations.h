#pragma once

#include <memory>
#include <string>
#include <vector>

namespace SmartMet
{
namespace Plugin
{
namespace WCS
{
/** Collection of case sensitive abbreviations
 */
class Abbreviations
{
 public:
  using Shared = std::shared_ptr<Abbreviations>;
  using Abbreviation = std::string;
  using Vector = std::vector<Abbreviation>;
  using Distance = int;

  Abbreviations();
  Abbreviations(const Vector& valuesVector);
  virtual ~Abbreviations();

  /** Get the abbreviation from a given distance.
   *  @return an empty string if not found
   */
  Abbreviation getAbbreviation(const Distance& distance) const;

  /** Get all the stored abbreviations.
   */
  const Vector& getAbbreviationVector() const;

  /** Get distance of an abbreviation from the begin of Vector
   *  Distance of the first is 0 and the last is length minus 1.
   *  @except WcsException if abbreviation not found.
   */
  Distance getDistance(const Abbreviation& abbreviation) const;

  /** Insert an abbreviation
   *  @except WcsException if abbreviation is already inserted
   *                       or abbreviation is empty
   *                       or abbreviation is multiple-part text.
   */
  void insert(const Abbreviation& value);

 private:
  Abbreviations(const Abbreviations& other) = delete;
  const Abbreviations& operator=(const Abbreviations& other) = delete;

  Vector mAbbreviations;
};
}
}
}
