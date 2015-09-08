#include "CPcompute.h"
#include "mysimulator.h"
#include "lammpscontroller.h"

LammpsOutput &CPCompute::output()
{
    return m_output;
}

bool CPCompute::isVector() const
{
    return m_isVector;
}

CPCompute::CPCompute(QObject *parent) : QObject(parent)
{
    m_output.m_compute = this;
}

QString CPCompute::identifier() const
{
    return m_identifier;
}

QString CPCompute::command() const
{
    return m_command;
}

MySimulator *CPCompute::simulator() const
{
    return m_simulator;
}

QStringList CPCompute::dependencies() const
{
    return m_dependencies;
}

double CPCompute::firstValue() const
{
    return m_values.at(0);
}

double CPCompute::secondValue() const
{
    return m_values.at(1);
}

double CPCompute::thirdValue() const
{
    return m_values.at(2);
}

double CPCompute::fourthValue() const
{
    return m_values.at(3);
}

double CPCompute::time() const
{
    return m_time;
}

QList<double> CPCompute::values() const
{
    return m_values;
}

void CPCompute::setIdentifier(QString identifier)
{
    if (m_identifier == identifier)
        return;

    m_identifier = identifier;
    emit identifierChanged(identifier);
}

void CPCompute::setCommand(QString command)
{
    if (m_command == command)
        return;

    m_command = command;
    emit commandChanged(command);
}

void CPCompute::setSimulator(MySimulator *simulator)
{
    if (m_simulator == simulator)
        return;

    m_simulator = simulator;
    simulator->addCompute(this);
    emit simulatorChanged(simulator);
}

void CPCompute::setDependencies(QStringList dependencies)
{
    if (m_dependencies == dependencies)
        return;

    m_dependencies = dependencies;
    emit dependenciesChanged(dependencies);
}

void CPCompute::setValues(QVector<double> values)
{
    m_time = m_simulator->simulationTime();
    m_values.clear();
    m_values = QList<double>::fromVector(values);

    emit valuesChanged(m_values);
    emit firstValueChanged(m_values.at(0));
    if(values.size() > 1) secondValueChanged(m_values.at(1));
    if(values.size() > 2) thirdValueChanged(m_values.at(2));
    if(values.size() > 3) fourthValueChanged(m_values.at(3));
}

void CPCompute::setIsVector(bool isVector)
{
    if (m_isVector == isVector)
        return;

    m_isVector = isVector;
    emit isVectorChanged(isVector);
}