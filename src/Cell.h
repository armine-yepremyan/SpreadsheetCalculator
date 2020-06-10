#ifndef CELL_H
#define CELL_H

#include <map>
#include <vector>
#include <memory>
#include <set>


/* ----------------------- Base Cell-----------------------*/
class Cell
{
public:
  typedef std::map<char, std::vector<std::shared_ptr<Cell>>> mSpreadsheet;

  enum Type
  {
    EMPTY,
    TEXT,
    NUMBER,
    EXPRESSION,
    REFERANCE
  };

  static const std::string errorFormat;
  static const std::string errorExpressionEvaluation;
  static const std::string errorReferenceCycling;
  static const std::string errorFormulaEntered;
  static bool isDataError(const std::string& cellData);

  static bool isDataEmpty(const std::string& cellData);
  static bool isDataText(const std::string& cellData);
  static bool isDataNumber(const std::string& cellData);
  static bool isDataCellReferance(const std::string& cellData);
  static bool isDataReferanceExpression(const std::string& cellData);
  static bool isDataSimpleExpression(const std::string& cellData);
  static Type getCellType(const std::string& cellData);

public:
  virtual ~Cell();
  virtual void calculate() = 0;
  virtual const Type getType() const = 0;
  virtual const std::string getValue() const = 0;
  static std::shared_ptr<Cell> createCell(const std::string&, const mSpreadsheet&, std::pair<char, int>);
};

/* ----------------------- EmptyCell -----------------------*/
class EmptyCell : public Cell
{
public:
  EmptyCell(const std::string& cellData, const mSpreadsheet& cells, Type type);
  ~EmptyCell();
  virtual void calculate();
  virtual const Type getType() const;
  virtual const std::string getValue() const;
private:
  const Type m_type;
};

/* ----------------------- TextCell -----------------------*/
class TextCell : public Cell
{
public:
  TextCell(const std::string& cellData, const mSpreadsheet& cells, Type type);
  ~TextCell();
  virtual void calculate();
  virtual const Type getType() const;
  virtual const std::string getValue() const;
private:
  std::string m_value;
  const Type m_type;
};

/* ----------------------- NumberCell -----------------------*/
class NumberCell : public Cell
{
public:
  NumberCell(const std::string& cellData, const mSpreadsheet& cells, Type type);
  ~NumberCell();
  virtual void calculate();
  virtual const Type getType() const;
  virtual const std::string getValue() const;
private:
  std::string m_value;
  const Type m_type;
};

/* ----------------------- ExpressionCell --------------------*/
class ExpressionCell : public Cell
{
public:
  ExpressionCell(const std::string& cellData, const mSpreadsheet& cells, Type type, std::pair<char, int>);
  ~ExpressionCell();
  virtual void calculate();
  virtual const Type getType() const;
  virtual const std::string getValue() const;

private:
  bool isParenthesis(const std::string& token);
  bool isOperator(const std::string& token);
  bool isMatchParentheses(const std::vector<std::string>&);

  const std::pair<char, int> getCellPosition() const;
  std::shared_ptr<Cell> getCell(char l, int row) const;
  std::vector<std::string> simpleExpressionTokenize(std::string& data);
  std::vector<std::string> referanceExpressionTokenize(std::pair<char, int> pos, std::string& data);

private:
  std::string m_value;
  std::string m_data;
  const Type m_type;
  bool m_isCalculated;
  const mSpreadsheet& m_cells;
  std::pair<char, int> m_cellPosition;
  std::vector<std::string> m_tokens;
  std::set<std::shared_ptr<Cell>> m_refBackup;
};


#endif // CELL_H
