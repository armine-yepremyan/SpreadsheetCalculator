#include <iostream>
#include <fstream>
#include <string>
#include <regex>

#include "./Cell.h"
#include "./SpreadsheetCalculator.h"

SpreadsheetCalculator::SpreadsheetCalculator(const char* inputFilename, const char* outputFilename)
      : m_inputFilename(inputFilename)
      , m_outputFilename(outputFilename)
{
  m_outputSheet = std::vector<std::string>(m_rows + 1, "");
}

SpreadsheetCalculator::~SpreadsheetCalculator()
{}

void SpreadsheetCalculator::readDataFromInputFile()
{
  std::vector<std::vector<std::string>> inputSheet;
  std::ifstream file;
  std::string str;
  try {
    file.open(m_inputFilename);
    bool isFileFirstLine = true;
    std::regex tReg("\\t");
    while (std::getline(file, str)) {
      str.erase(str.length() - 1);
      auto const vec = std::vector<std::string>(
        std::sregex_token_iterator{begin(str), end(str), tReg, -1},
        std::sregex_token_iterator{});

      // validation input data format
      if (isFileFirstLine) {
        if (vec.size() != 2
          || !Cell::isDataNumber(vec[0])
          || !Cell::isDataNumber(vec[1]))
        {
            throw std::runtime_error("Err: Invalid file content!");
        }
        m_rows = std::stoi(vec[0]);
        m_columns = std::stoi(vec[1]);
        isFileFirstLine = false;
      } else {
        if (vec.size() != m_columns) {
          throw std::runtime_error("Err: Invalid file content!");
        }
      }
      inputSheet.push_back(vec);
    }
    file.close();
  } catch (std::ifstream::failure e) {
    std::cerr << "Err: Exception opening/reading/closing input file\n";
  }

  // input data in to std::map<char, std::vector<std::shared_ptr<Cell>>> m_cells
  {
  int i = 0;
  char l = 'A';
  std::vector<std::shared_ptr<Cell>> vCell;
  while(i < m_columns) {
    for (int j = 1; j < (int)inputSheet.size(); ++j) {
      vCell.push_back(Cell::createCell(inputSheet[j][i], m_cells, std::make_pair(l, j)));
    }
    m_cells[l] = vCell;
    vCell.clear();
    ++l;
    ++i;
  }
  }
}

void SpreadsheetCalculator::calculate()
{
  m_outputSheet[0] += "  ";
  for (auto it : m_cells) {
    m_outputSheet[0] += (it.first + std::string("\t"));
    int j = 1;
    for (auto cell : it.second) {
      std::shared_ptr<Cell> tmp = cell;
      tmp->calculate();
      if (m_outputSheet[j].empty()) {
        m_outputSheet[j] = std::to_string(j) + ' ';
      }
      m_outputSheet[j] = (m_outputSheet[j] + tmp->getValue() + std::string("\t"));
      ++j;
    }
  }
}

void SpreadsheetCalculator::writeCalculatedDataToOutputFile()
{
  calculate();
  std::ofstream file;
  try {
    file.open(m_outputFilename);
    for (const auto& line : m_outputSheet) {
      file << line + '\n';
    }
    file.close();
  } catch (const std::ios_base::failure e) {
    std::cerr << "Err: Exception opening/writing/closing output file\n";
  }
}
