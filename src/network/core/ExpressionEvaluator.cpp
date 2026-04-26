/**
 * @file ExpressionEvaluator.cpp
 * @brief ExprTk-based expression evaluator for calculated columns
 */

#include "ExpressionEvaluator.h"
#include <QRegularExpression>
#include <cmath>

namespace dn::core {

ExpressionEvaluator::ExpressionEvaluator()
    : m_impl(new Impl())
{
}

ExpressionEvaluator::~ExpressionEvaluator()
{
    delete m_impl;
}

void ExpressionEvaluator::setVariable(const QString& name, const QVariant& value)
{
    double dValue = 0.0;
    
    if (!value.isNull()) {
        if (value.typeId() == QMetaType::Int || value.typeId() == QMetaType::LongLong) {
            dValue = value.toLongLong();
        } else if (value.typeId() == QMetaType::Double || value.typeId() == QMetaType::Float) {
            dValue = value.toDouble();
        } else if (value.typeId() == QMetaType::Bool) {
            dValue = value.toBool() ? 1.0 : 0.0;
        } else {
            bool ok;
            dValue = value.toString().toDouble(&ok);
            if (!ok) {
                dValue = 0.0;
            }
        }
    }
    
    QString safeName = name;
    safeName.replace(QRegularExpression("[^a-zA-Z0-9_]"), "_");
    if (!safeName.isEmpty() && !safeName[0].isLetter()) {
        safeName = "v_" + safeName;
    }
    
    std::string stdName = safeName.toStdString();
    
    qDebug() << "setVariable:" << QString::fromStdString(stdName) << "=" << dValue;
    
    m_impl->symTable.remove_variable(stdName);
    m_impl->varValues[stdName] = dValue;
    m_impl->symTable.add_variable(stdName, m_impl->varValues[stdName]);
}

void ExpressionEvaluator::setVariables(const QMap<QString, QVariant>& variables)
{
    for (auto it = variables.constBegin(); it != variables.constEnd(); ++it) {
        setVariable(it.key(), it.value());
    }
}

void ExpressionEvaluator::clear()
{
    m_impl->varValues.clear();
    m_impl->expr.release();
}

QVariant ExpressionEvaluator::evaluate(const QString& expression, bool* ok) const
{
    if (ok) *ok = false;
    m_impl->error.clear();
    
    m_impl->expr.release();
    m_impl->expr.register_symbol_table(m_impl->symTable);
    
    std::string exprStr = expression.toStdString();
    qDebug() << "Evaluating expression:" << QString::fromStdString(exprStr);
    
    if (!m_impl->parser.compile(exprStr, m_impl->expr)) {
        std::string error = m_impl->parser.error();
        m_impl->error = QString::fromStdString(error);
        qDebug() << "ExprTk error:" << m_impl->error;
        return QVariant();
    }
    
    double result = m_impl->expr.value();
    qDebug() << "Result:" << result;
    
    if (std::isnan(result)) {
        return QVariant();
    }
    
    if (result == std::floor(result)) {
        if (ok) *ok = true;
        return static_cast<qint64>(result);
    }
    
    if (ok) *ok = true;
    return result;
}

} // namespace dn::core