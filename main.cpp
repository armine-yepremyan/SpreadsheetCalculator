#include <iostream>
#include <stdexcept>

#include "./src/Cell.h"
#include "./src/SpreadsheetCalculator.h"

int main(int argc, char *argv[]) {
  if (argc != 3) {
    return 1;
  }
  SpreadsheetCalculator calculater(argv[1], argv[2]);
  calculater.readDataFromInputFile();
  calculater.writeCalculatedDataToOutputFile();
  return 0;
}
