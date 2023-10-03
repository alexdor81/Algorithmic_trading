#pragma once

#include "date_time.h"

struct Data {
  Data(DateTime date, double price) : date_(date), price_(price) {}

  DateTime date_;
  double price_;
};
