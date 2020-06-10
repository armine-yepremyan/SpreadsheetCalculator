#ifndef SPREADSHEETCALCULATOR_H
#define SPREADSHEETCALCULATOR_H

#include <map>
#include <memory>
#include <vector>

class Cell;

class SpreadsheetCalculator
{
  typedef std::map<char, std::vector<std::shared_ptr<Cell>>> mSpreadsheet;
public:
  SpreadsheetCalculator(const char* inputFilename, const char* outputFilename);
  ~SpreadsheetCalculator();
  void readDataFromInputFile();
  void writeCalculatedDataToOutputFile();

private:
  void calculate();
private:
  int m_rows;
  int m_columns;
  const char* m_inputFilename;
  const char* m_outputFilename;
  mSpreadsheet m_cells;

  std::vector<std::string> m_outputSheet;
};

#endif // SPREADSHEETCALCULATOR_H
