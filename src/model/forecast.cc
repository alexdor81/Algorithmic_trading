#include "forecast.h"

void Forecast::LoadData(const std::string &file_path) {
  std::ifstream file(file_path);

  if (!file.is_open()) throw std::invalid_argument("Ошибка при открытии файла");

  data_.clear();
  std::string line;
  std::getline(file, line);

  while (std::getline(file, line)) {
    std::istringstream sstream(line);
    std::string date;
    std::getline(sstream, date, ',');
    DateTime time_point(date);
    double price;
    sstream >> price;
    data_.emplace_back(time_point, price);
  }
  file.close();
}

void Forecast::Interpolation(int count) {
  time_t period =
      data_.back().date_.ToTime_t() - data_.front().date_.ToTime_t();
  std::vector<time_t> dates = DefineDates(count, period);
  std::array<std::vector<long double>, 4> coeffs = CalcInterpolationCoeffs();
  forecast_.clear();
  for (size_t i = 0; i < dates.size(); ++i)
    forecast_.emplace_back(
        dates[i], InterpolatePrice(DefineIndex(dates[i]), dates[i], coeffs));
}

void Forecast::Approximation(int count, int days, int degree) {
  time_t period =
      data_.back().date_.AddDays(days) - data_.front().date_.ToTime_t();
  std::vector<time_t> dates = DefineDates(count, period);
  std::vector<long double> coeffs = CalcApproximationCoeffs(degree);
  forecast_.clear();
  for (size_t i = 0; i < dates.size(); ++i)
    forecast_.emplace_back(dates[i], ApproximatePrice(dates[i], coeffs));
}

void Forecast::InterpolatePriceByDate(time_t date) {
  std::array<std::vector<long double>, 4> coeffs = CalcInterpolationCoeffs();
  forecast_price_ = InterpolatePrice(DefineIndex(date), date, coeffs);
}

void Forecast::ApproximatePriceByDate(time_t date, int degree) {
  std::vector<long double> coeffs = CalcApproximationCoeffs(degree);
  forecast_price_ = ApproximatePrice(date, coeffs);
}

std::vector<time_t> Forecast::DefineDates(int count, time_t period) {
  time_t interval = period / (count - 1);
  std::vector<time_t> dates;
  time_t date_i = data_.front().date_.ToTime_t();
  for (int i = 0; i < count; ++i) {
    dates.push_back(date_i);
    date_i += interval;
  }
  return dates;
}

std::vector<long double> Forecast::SolveSle(matrix &sle) {
  size_t size = sle.size();
  for (size_t k = 0; k < size; k++) {
    for (size_t i = k + 1; i < size; i++) {
      long double factor = sle[i][k] / sle[k][k];
      for (size_t j = k; j < size; j++) sle[i][j] -= factor * sle[k][j];
      sle[i][size] -= factor * sle[k][size];
    }
  }

  std::vector<long double> res = std::vector<long double>(size);
  for (size_t i = size - 1; i != 0; i--) {
    long double s = 0;
    for (size_t j = i; j < size; j++) s += sle[i][j] * res[j];
    res[i] = (sle[i][size] - s) / sle[i][i];
  }
  return res;
}

std::vector<long double> Forecast::CalcApproximationCoeffs(int degree) {
  matrix sle(degree + 1, std::vector<long double>(degree + 2));
  std::vector<long double> coeffs = std::vector<long double>(degree + 1);

  for (int i = 0; i < degree + 1; i++)
    for (int j = 0; j < degree + 1; j++)
      for (int k = 0; k < data_.size(); k++)
        sle[i][j] += powl(data_[k].date_.ToDouble(), i + j);

  for (int i = 0; i < degree + 1; i++)
    for (int j = 0; j < data_.size(); j++)
      sle[i][degree + 1] +=
          powl(data_[j].date_.ToDouble(), i) * data_[j].price_;

  for (int k = 0; k < degree + 1; k++) {
    for (int i = k + 1; i < degree + 1; i++) {
      long double factor = sle[i][k] / sle[k][k];
      for (int j = k; j < degree + 1; j++) sle[i][j] -= factor * sle[k][j];
      sle[i][degree + 1] -= factor * sle[k][degree + 1];
    }
  }
  for (int i = degree; i >= 0; i--) {
    long double sum = 0;
    for (int j = i; j < degree + 1; j++) sum += sle[i][j] * coeffs[j];
    coeffs[i] = (sle[i][degree + 1] - sum) / sle[i][i];
  }
  return coeffs;
}

std::array<std::vector<long double>, 4> Forecast::CalcInterpolationCoeffs() {
  std::vector<long double> dx(data_.size());  // date intervals
  std::vector<long double> dy(data_.size());  // price intervals
  for (size_t i = 1; i < data_.size(); ++i) {
    dx[i] = data_[i].date_.ToDouble() - data_[i - 1].date_.ToDouble();
    dy[i] = data_[i].price_ - data_[i - 1].price_;
  }
  matrix sle(data_.size(), std::vector<long double>(data_.size() + 1));
  sle[0][0] = 1;
  sle[data_.size() - 1][data_.size() - 1] = 1;
  for (size_t i = 1; i < sle.size() - 1; ++i) {
    sle[i][i - 1] = dx[i];
    sle[i][i] = 2 * (dx[i] + dx[i + 1]);
    sle[i][i + 1] = dx[i + 1];
    sle[i].back() = 3.0 * (dy[i + 1] / dx[i + 1] - dy[i] / dx[i]);
  }
  std::array<std::vector<long double>, 4> coeffs;
  coeffs[C] = SolveSle(sle);
  coeffs[A] = std::vector<long double>(data_.size());
  coeffs[B] = std::vector<long double>(data_.size());
  coeffs[D] = std::vector<long double>(data_.size());
  coeffs[A].front() = data_.front().price_;
  for (size_t i = 1; i < data_.size(); ++i) {
    coeffs[A][i] = data_[i].price_;
    coeffs[B][i] =
        dy[i] / dx[i] + (2.0 * coeffs[C][i] + coeffs[C][i - 1]) / 3.0 * dx[i];
    coeffs[D][i] = (coeffs[C][i] - coeffs[C][i - 1]) / (3.0 * dx[i]);
  }
  return coeffs;
}

int Forecast::DefineIndex(time_t date) {
  int index = 0;
  while (date > data_[index].date_.ToTime_t()) ++index;
  return index;
}

double Forecast::InterpolatePrice(
    int index, time_t date,
    const std::array<std::vector<long double>, 4> &coeffs) {
  time_t delta = date - data_[index].date_.ToTime_t();
  return coeffs[A][index] * powl(delta, A) + coeffs[B][index] * powl(delta, B) +
         coeffs[C][index] * powl(delta, C) + coeffs[D][index] * powl(delta, D);
}

double Forecast::ApproximatePrice(time_t date,
                                  std::vector<long double> coeffs) {
  double price = 0.0;
  for (size_t j = 0; j < coeffs.size(); ++j) price += coeffs[j] * powl(date, j);
  return price;
}

time_t Forecast::GetMaxDate() const { return data_.back().date_.ToTime_t(); }

time_t Forecast::GetMinDate() const { return data_.front().date_.ToTime_t(); }

double Forecast::GetForecastPrice() const { return forecast_price_; }

const std::vector<Data> &Forecast::GetForecast() const { return forecast_; }

const std::vector<Data> &Forecast::GetData() const { return data_; }
