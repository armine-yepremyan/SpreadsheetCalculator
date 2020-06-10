#include <regex>
#include <iostream>

#include "./Cell.h"
#include "./SpreadsheetCalculator.h"


/* ----------------------- Base Cell-----------------------*/
const std::string Cell::errorFormat = "#UNKNOWN_FORMAT";
const std::string Cell::errorExpressionEvaluation = "#TEXT?";
const std::string Cell::errorReferenceCycling = "#CIRCULAR_REF";
const std::string Cell::errorFormulaEntered = "#WRONG_FORMULA_TYPE";

bool Cell::isDataError(const std::string& cellData)
{
    return cellData == errorFormat
          || cellData == errorExpressionEvaluation
          || cellData == errorReferenceCycling
          || cellData == errorFormulaEntered;
}

Cell::~Cell()
{}

std::shared_ptr<Cell> Cell::createCell(const std::string& cellData, const mSpreadsheet& cells, std::pair<char, int> pos)
{
    switch (getCellType(cellData)) {
      case EMPTY:
        return std::shared_ptr<Cell>(new EmptyCell(cellData, cells, EMPTY));
      case NUMBER:
        return std::shared_ptr<Cell>(new NumberCell(cellData, cells, NUMBER));
      case TEXT:
        return std::shared_ptr<Cell>(new TextCell(cellData, cells, TEXT));
      case EXPRESSION:
        return std::shared_ptr<Cell>(new ExpressionCell(cellData, cells, EXPRESSION, pos));
      case REFERANCE:
        return std::shared_ptr<Cell>(new ExpressionCell(cellData, cells, REFERANCE, pos));
      default: {
        throw "Error: Invalid Cell Data Type!";
      }
    }
}


bool Cell::isDataEmpty(const std::string& cellData)
{
    return cellData.empty();
}

bool Cell::isDataText(const std::string& cellData)
{
    for (auto i : cellData)
    {
      if (!std::isprint(i)) {
        return false;
      }
    }
    return true;
}


bool Cell::isDataNumber(const std::string& cellData)
{
    return std::regex_match (cellData, std::regex("^[-+]?[0-9]*$"));
}

bool Cell::isDataCellReferance(const std::string& cellData)
{
    return std::regex_match (cellData, std::regex("([a-zA-Z][1-9][0-9]{0,2})"));
}

bool Cell::isDataReferanceExpression(const std::string& cellData)
{
    if (cellData[0] == '=') {
      return !std::regex_match (cellData, std::regex("^=[0-9+*-/^()., ]+$"));
    }
    return false;
}

bool Cell::isDataSimpleExpression(const std::string& cellData)
{
    return std::regex_match (cellData, std::regex("^=[0-9+*-/^()., ]+$"));
}


Cell::Type Cell::getCellType(const std::string& cellData)
{
    if (isDataSimpleExpression(cellData)) {
      return EXPRESSION;
    }
    if (isDataReferanceExpression(cellData)) {
      return REFERANCE;
    }
    if (isDataEmpty(cellData)) {
      return EMPTY;
    }
    if (isDataNumber(cellData)) {
      return NUMBER;
    }
    if (isDataText(cellData)) {
      return TEXT;
    }
}


/* ----------------------- EmptyCell -----------------------*/
EmptyCell::EmptyCell(const std::string& cellData, const mSpreadsheet& cells, Type type)
    : m_type(type)
{}

EmptyCell::~EmptyCell()
{}

void EmptyCell::calculate()
{
  //Nothing to do
}

const Cell::Type EmptyCell::getType() const
{
    return m_type;
}

const std::string EmptyCell::getValue() const
{
    return std::string("");
}

/* ----------------------- TextCell -----------------------*/
TextCell::TextCell(const std::string& cellData, const mSpreadsheet& cells, Type type)
    : m_value(cellData)
    , m_type(type)
{}

TextCell::~TextCell()
{}

void TextCell::calculate()
{
    if (m_value[0] != '\'') {
      m_value = errorFormat;
    } else {
      m_value.erase(0, 1);
    }
}

const Cell::Type TextCell::getType() const
{
    return m_type;
}

const std::string TextCell::getValue() const
{
    return m_value;
}

/* ----------------------- NumberCell -----------------------*/
NumberCell::NumberCell(const std::string& cellData, const mSpreadsheet& cells, Type type)
    : m_value(cellData)
    , m_type(type)
{}

NumberCell::~NumberCell()
{}

void NumberCell::calculate()
{
    m_value.erase(0, m_value.find_first_not_of('0'));
}

const Cell::Type NumberCell::getType() const
{
    return m_type;
}

const std::string NumberCell::getValue() const
{
    return m_value;
}

/* ----------------------- ExpressionCell --------------------*/
ExpressionCell::ExpressionCell(const std::string& cellData, const mSpreadsheet& cells, Type type, std::pair<char, int> pos)
    : m_value(cellData)
    , m_data(cellData)
    , m_type(type)
    , m_isCalculated(false)
    , m_cells(cells)
    , m_cellPosition(pos)
{}

ExpressionCell::~ExpressionCell()
{}

void ExpressionCell::calculate()
{
    std::vector<std::string> tokens = m_type != EXPRESSION
                                  ? referanceExpressionTokenize(getCellPosition(), m_data)
                                  : simpleExpressionTokenize(m_data);
    if (m_isCalculated) {
      return;
    }
    m_isCalculated =  true;

    if (!isMatchParentheses(tokens)) {
      return;
    }

    std::stack<std::string> st;
    for (const auto token : m_tokens) {
      if (!isOperator(token)) {
        st.push(token);
      } else {
        int result = 0;
        const std::string val2 = st.top();
        st.pop();
        const int d2 = std::stoi(val2);
        if (!st.empty()) {
          const std::string val1 = st.top();
          st.pop();
          const int d1 = std::stoi(val1);
          //Get the result
          if (0 == d2 && token == "/") {
            m_value = "#ERROR_NUM";
            return;
          }
          result = token == "+" ? d1 + d2 :
                   token == "-" ? d1 - d2 :
                   token == "*" ? d1 * d2 :
                                  d1 / d2;
        } else {
          result = token == "-" ? (d2 * -1) : d2;
        }
        std::ostringstream s;
        s << result;
        st.push(s.str());
      }
    }
    m_value = st.top();
}

const Cell::Type ExpressionCell::getType() const
{
    return m_type;
}

const std::string ExpressionCell::getValue() const
{
    return m_value;
}

bool ExpressionCell::isParenthesis(const std::string& token)
{
    return token == "(" || token == ")";
}

bool ExpressionCell::isOperator(const std::string& token)
{
    return token == "+" || token == "-" || token == "*" || token == "/";
}

bool ExpressionCell::isMatchParentheses(const std::vector<std::string>& inputTokens)
{
    // to check that the parentheses matched and collect tokens
    std::stack<std::string> stack;
    for (auto i : inputTokens) {
        const std::string token = i;
        if (isOperator(token)) {
            const std::string operator1 = token;
            if (!stack.empty()) {
                std::string operator2 = stack.top();
                while (isOperator(operator2)) {
                    stack.pop();
                    m_tokens.push_back(operator2);

                    if(stack.empty()) break;
                    operator2 = stack.top();
                }
            }
            stack.push(operator1);
        }
        else if (token == "(") {
            // Push token to top of the stack
            stack.push( token );
        }
        else if (token == ")") {
            if (stack.empty()) {
              m_value = errorFormulaEntered;
              m_isCalculated = true;
              return false;
            }
            std::string topToken  = stack.top();
            while (topToken != "(") {
                m_tokens.push_back(topToken );
                stack.pop();
                if (stack.empty()) break;
                topToken = stack.top();
            }
            if (!stack.empty()) stack.pop();
            if (topToken != "(") {
              m_value = errorFormulaEntered;
              m_isCalculated = true;
              return false;
            }
        }
        else {
            m_tokens.push_back(token);
        }
    }
    // While there are still operator tokens in the stack:
    while (!stack.empty()) {
        const std::string stackToken = stack.top();
        if (isParenthesis(stackToken)) {
            m_value = errorFormulaEntered;
            m_isCalculated = true;
            return false;
        }
        m_tokens.push_back(stackToken);
        stack.pop();
    }
    return true;
}

const std::pair<char, int> ExpressionCell::getCellPosition() const
{
    return m_cellPosition;
}

std::shared_ptr<Cell> ExpressionCell::getCell(char l, int row) const
{
    return m_cells.find(l)->second[row - 1];
}

std::vector<std::string> ExpressionCell::simpleExpressionTokenize(std::string& data)
{
    std::vector<std::string> tokens;
    std::string str = "";
    data.erase(0, 1);
    for (auto i : data) {
      const std::string token(1, i);

      if (isOperator(token) || isParenthesis(token)) {
          if (!str.empty()) {
            if ((m_type == EXPRESSION) && !isDataNumber(str)) {
              m_value = errorExpressionEvaluation;
              m_isCalculated = true;
              return tokens;
            }
            tokens.push_back(str);
          }
          str = "";
          tokens.push_back(token);
      } else {
          if (!token.empty() && token != " ") {
              str.append(token);
          }
      }
    }
    if (!str.empty() && str != " ") {
      tokens.push_back(str);
    }
    return tokens;
}

std::vector<std::string> ExpressionCell::referanceExpressionTokenize(std::pair<char, int> pos, std::string& data)
{
    std::vector<std::string> resultTokens;
    if (isDataNumber(data)) {
      m_refBackup.clear();
      resultTokens.push_back(data);
      return resultTokens;
    }
    if (isDataError(data)) {
      m_value = data;
      m_isCalculated = true;
      m_refBackup.clear();
      resultTokens.push_back(data);
      return resultTokens;
    }
    std::transform(begin(data), end(data), begin(data), [](unsigned char c){ return std::toupper(c); });

    auto tokens = simpleExpressionTokenize(data);

    for (auto elem : tokens) {
      if (!isDataCellReferance(elem) && !isOperator(elem) && !isParenthesis(elem) && !isDataNumber(elem)) {
        m_value = errorExpressionEvaluation;
        resultTokens.clear();
        resultTokens.push_back(errorExpressionEvaluation);
        m_isCalculated = true;
        break;
      }
      // check if elem is in map
      auto it = m_cells.find(elem[0]);
      if (it != m_cells.end()) {
        std::string sRow = elem;
        int row = std::stoi(sRow.erase(0, 1));
        if (row > it->second.size()) {
          m_value = errorExpressionEvaluation;
          resultTokens.clear();
          resultTokens.push_back(errorExpressionEvaluation);
          m_isCalculated = true;
          break;
        }
        std::shared_ptr<Cell> tmpCell = getCell(elem[0], row);
        std::string str = tmpCell->getValue();

        // Calculated and value is error
        if (isDataError(str)) {
         m_value = str;
         m_isCalculated = true;
         resultTokens.clear();
         resultTokens.push_back(data);
         break;
       }

       // Type is number
       else if (tmpCell->getType() == NUMBER) {
         resultTokens.push_back(str);
       }

       // Type is text
       else if (tmpCell->getType() == TEXT) {
         m_value = errorExpressionEvaluation;
         resultTokens.clear();
         resultTokens.push_back(errorExpressionEvaluation);
         m_isCalculated = true;
         break;
       }

       // Type is simple expression
       else if (tmpCell->getType() == EXPRESSION) {
         tmpCell->calculate();
         if (isDataError(tmpCell->getValue())) {
           m_value = tmpCell->getValue();
           m_isCalculated = true;
           resultTokens.clear();
           resultTokens.push_back(tmpCell->getValue());
           break;
         }
         resultTokens.push_back(tmpCell->getValue());
       }

       // Type is empty = '0'
       else if (tmpCell->getType() == EMPTY) {
         resultTokens.push_back("0");
       }

       else {
         auto it = m_refBackup.find(tmpCell);
         if (it != m_refBackup.end()) {
           m_value = errorReferenceCycling;
           m_isCalculated = true;
           m_refBackup.clear();
           resultTokens.clear();
           resultTokens.push_back(errorReferenceCycling);
           return resultTokens;
         } else {
           m_refBackup.insert(tmpCell);
         }

         std::shared_ptr<ExpressionCell> exprtmp = std::dynamic_pointer_cast<ExpressionCell>(tmpCell);
         std::vector<std::string> tmp = referanceExpressionTokenize(exprtmp->getCellPosition(), str);
         resultTokens.assign(begin(tmp), end(tmp));
       }
      } else {
        if (isDataNumber(elem) || isOperator(elem) || isParenthesis(elem)) {
          resultTokens.push_back(elem);
        }
        else {
          m_value = errorExpressionEvaluation;
          resultTokens.clear();
          resultTokens.push_back(errorExpressionEvaluation);
          m_isCalculated = true;
          break;
        }
      }
    }
    m_refBackup.clear();
    return resultTokens;
}
