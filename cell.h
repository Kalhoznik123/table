#pragma once

#include "common.h"
#include "formula.h"

#include <functional>
#include <unordered_set>
#include <set>
#include <optional>

class Sheet;

class Cell : public CellInterface {
public:
        Cell(Sheet& sheet);
        ~Cell();

        void Set(std::string text);
        void Clear();

        Value GetValue() const override;
        std::string GetText() const override;
        std::vector<Position> GetReferencedCells() const override;

        bool IsReferenced() const;

private:

        class Impl{
        public:
            virtual ~Impl() = default;

            virtual Value GetValue() const  = 0;

            virtual std::string GetText() const  = 0;

        };

        class EmptyImpl : public Impl{
        public:
            Value GetValue() const override{
                return std::move("");
            }

            std::string GetText() const override{
                return "";
            }

        };

        class FormulaImpl :public  Impl{
        public:
            FormulaImpl(std::string expression,const SheetInterface& sheet)
                :sheet_(sheet)
                ,formula_(ParseFormula(std::move(expression))){
            }

            Value GetValue() const override{
                const auto result = formula_->Evaluate(sheet_);

                Value value;
                if(auto* ptr = std::get_if<double>(&result)){
                    value =*ptr;
                }else if(auto* ptr = std::get_if<FormulaError>(&result)){
                    value = *ptr;
                }

                return value;
            }

            std::string GetText() const override{
                auto result =formula_->GetExpression();
                return "=" + result;
            }

            std::vector<Position> GetReferencedCells() const {
                return formula_->GetReferencedCells();
            }

        private:
            const SheetInterface& sheet_;
            std::unique_ptr<FormulaInterface> formula_;
        };

        class TextImpl : public Impl{
        public:
            TextImpl(std::string expression)
                :expression_(std::move(expression))
            {}


            Value GetValue() const override{
                if(expression_[0] == ESCAPE_SIGN){
                    return expression_.substr(1,expression_.npos);
                }
                return std::move(expression_);
            }

            std::string GetText() const override{
                return std::move(expression_);
            }


        private:
            std::string expression_;
        };

        void CheckLoopReferences(const std::vector<Position>& leaves, const CellInterface* root);
        void UpdateRef();
        void InvalidateCach();

private:
        Sheet& sheet_;
        std::unique_ptr<Impl> impl_;
        std::set<CellInterface*> child_nodes_;
        std::set<CellInterface*> parent_nodes_;
        mutable  std::optional<Value> prev_value;

};
