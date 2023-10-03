#pragma once

#include <array>
#include <cmath>
#include <fstream>
#include <vector>

#include "data.h"

class Forecast {
  using matrix = std::vector<std::vector<long double>>;

 public:
  void LoadData(const std::string &);
  void Interpolation(int);
  void Approximation(int, int, int);
  void InterpolatePriceByDate(time_t);
  void ApproximatePriceByDate(time_t, int);

  time_t GetMaxDate() const;
  time_t GetMinDate() const;

  double GetForecastPrice() const;
  const std::vector<Data> &GetForecast() const;
  const std::vector<Data> &GetData() const;

 private:
  std::vector<time_t> DefineDates(int, time_t);
  std::vector<long double> SolveSle(matrix &);
  std::vector<long double> CalcApproximationCoeffs(int);
  std::array<std::vector<long double>, 4> CalcInterpolationCoeffs();
  int DefineIndex(time_t);
  double InterpolatePrice(int, time_t,
                          const std::array<std::vector<long double>, 4> &);
  double ApproximatePrice(time_t, std::vector<long double>);

  enum InterpolationCoefficients { A, B, C, D };

  double forecast_price_ = 0.0;
  std::vector<Data> forecast_;
  std::vector<Data> data_;
};
