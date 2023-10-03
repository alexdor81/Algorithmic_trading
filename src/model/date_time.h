#pragma once

#include <chrono>
#include <iomanip>
#include <sstream>
#include <string>

class DateTime {
 public:
  DateTime(time_t);
  DateTime(std::string &);

  std::string ToString() const;
  time_t ToTime_t() const;
  time_t AddDays(int) const;
  long double ToDouble() const;
  bool isValid() const;
  std::chrono::time_point<std::chrono::system_clock> &GetValue();

 private:
  std::chrono::time_point<std::chrono::system_clock> date_;
  bool isValid_;
};
