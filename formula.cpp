#include "formula.h"

#include "FormulaAST.h"

#include <algorithm>
#include <cassert>
#include <cctype>
#include <sstream>
#include <memory>

using namespace std::literals;

std::ostream& operator<<(std::ostream& output, FormulaError fe) {
   return output << fe.ToString();
}

namespace {
class Formula : public FormulaInterface {
public:
    // Реализуйте следующие методы:
    explicit Formula(std::string expression)
        : ast_(ParseFormulaAST(std::move(expression))){
        
    }
    Value Evaluate(const SheetInterface& sheet ) const override{

        auto func_lamda = [&sheet](const Position& pos){
          if(!pos.IsValid()){
              throw FormulaError(FormulaError::Category::Ref);
          }
            const CellInterface* cell = sheet.GetCell(pos);
            CellInterface::Value value = cell->GetValue();

            if(const auto it = std::get_if<double>(&value)){
                return *it;
            }else if(const auto it = std::get_if<std::string>(&value)){
                double  res = 0;
                try {
                    res = std::stod(*it);
                } catch (...) {
                    throw FormulaError(FormulaError::Category::Value);
                }
                return  res;
            }else {
                throw std::get<FormulaError>(value);
            }
        };

        Value result;
        try {
            result  = ast_.Execute(func_lamda);
        } catch (FormulaError& exep) {
            result = exep;
        }
        return  result;
    }
    std::string GetExpression() const override {
        std::ostringstream out;
        ast_.PrintFormula(out);
        std::string result = out.str();

        return result;
    }

    std::vector<Position> GetReferencedCells() const override{
        auto cells = ast_.GetCells();
        std::vector<Position> result{std::make_move_iterator(cells.begin()),std::make_move_iterator(cells.end())};
        std::sort(result.begin(),result.end());
        return result;
    }
private:
    FormulaAST ast_;
};

}  // namespace

std::unique_ptr<FormulaInterface> ParseFormula(std::string expression) {
    return std::make_unique<Formula>(std::move(expression));
}
