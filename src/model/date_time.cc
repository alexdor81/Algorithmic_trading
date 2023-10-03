#include "date_time.h"

DateTime::DateTime(std::string &date) {
  tm tm{};
  std::istringstream line(date);
  line >> std::get_time(&tm, "%Y-%m-%d");
  if (!line.fail()) {
    time_t time = std::mktime(&tm);
    date_ = std::chrono::system_clock::from_time_t(time);
    isValid_ = true;
  } else {
    isValid_ = false;
  }
}

DateTime::DateTime(time_t time)
    : date_(std::chrono::system_clock::from_time_t(time)), isValid_(true) {}

std::string DateTime::ToString() const {
  if (!isValid_) return "(null)";
  time_t time = std::chrono::system_clock::to_time_t(date_);
  tm tm = *std::localtime(&time);
  std::ostringstream line;
  line << std::put_time(&tm, "%Y-%m-%d");
  return line.str();
}

long double DateTime::ToDouble() const {
  return std::chrono::duration<long double>(date_.time_since_epoch()).count();
}

time_t DateTime::ToTime_t() const {
  return std::chrono::system_clock::to_time_t(date_);
}

time_t DateTime::AddDays(int days) const {
  return std::chrono::system_clock::to_time_t(date_ +
                                              std::chrono::hours(days * 24));
}

bool DateTime::isValid() const { return isValid_; }

std::chrono::time_point<std::chrono::system_clock> &DateTime::GetValue() {
  return date_;
}
