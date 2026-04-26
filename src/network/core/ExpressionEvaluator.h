/**
 * @file ExpressionEvaluator.h
 * @brief ExprTk-based expression evaluator for calculated columns
 */

#ifndef EXPRESSIONEVALUATOR_H
#define EXPRESSIONEVALUATOR_H

#include <QString>
#include <QVariant>
#include <QMap>
#include <QStringList>
#include <unordered_map>
#include "exprtk.hpp"

namespace dn::core {

class ExpressionEvaluator {
public:
    ExpressionEvaluator();
    ~ExpressionEvaluator();
    
    void setVariable(const QString& name, const QVariant& value);
    void setVariables(const QMap<QString, QVariant>& variables);
    void clear();
    
    QVariant evaluate(const QString& expression, bool* ok = nullptr) const;
    QString lastError() const { return m_error; }
    
private:
    struct Impl {
        exprtk::symbol_table<double> symTable;
        exprtk::expression<double> expr;
        exprtk::parser<double> parser;
        std::unordered_map<std::string, double> varValues;
        QString error;
    };
    Impl* m_impl;
    mutable QString m_error;
};

} // namespace dn::core

#endif // EXPRESSIONEVALUATOR_H