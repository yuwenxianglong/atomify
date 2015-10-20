#include "nvt.h"
#include "lammpscontroller.h"
#include <fix_nvt.h>
#include <QString>

class FixNHHack : public FixNH {
public:
    double target() { return t_target; }
    void setTargets(double targetValue) {
        t_target = targetValue;
        t_start = targetValue;
        t_stop = targetValue;
    }
};

NVT::NVT(QObject *parent) : SimulatorControl(parent)
{

}

double NVT::targetTemperature() const
{
    return m_targetTemperature;
}

void NVT::setTargetTemperature(double targetTemperature)
{
    if (m_targetTemperature == targetTemperature)
        return;

    m_targetTemperature = targetTemperature;
    setDirty(true);
    emit targetTemperatureChanged(targetTemperature);
}


void NVT::synchronizeLammps(LAMMPSController *lammpsController)
{
    FixNVT *fix = lammpsController->findFixByType<FixNVT>();
    if(fix) {
        FixNH *fixNH = dynamic_cast<FixNH*>(fix);
        FixNHHack *fixHack = reinterpret_cast<FixNHHack*>(fixNH);
        if(fixHack) {
            if(m_dirty) {
                m_dirty = false;

                if(!m_enabled) {
                    QList<QString> disableCommands;
                    disableCommands.push_back(QString("unfix %1").arg(fix->id));
                    disableCommands.push_back("fix nve all nve");
                    lammpsController->scriptHandler()->addCommandsToTop(disableCommands, CommandInfo(CommandInfo::Type::SingleCommand));
                    return;
                }

                if(m_targetTemperature != fixHack->target()) {
                    fixHack->setTargets(m_targetTemperature);
                }
            } else {
                if(m_targetTemperature != fixHack->target()) {
                    m_targetTemperature = fixHack->target();
                    emit targetTemperatureChanged(m_targetTemperature);
                }
                if(!m_enabled) {
                    m_enabled = true;
                    emit enabledChanged(m_enabled);
                }
            }
        }
    } else {
        if(m_dirty && m_enabled) {
            lammpsController->disableAllEnsembleFixes();
            QString command = QString("fix nvt all nvt temp %1 %1 1.0").arg(m_targetTemperature);
            lammpsController->scriptHandler()->addCommandToTop(command, CommandInfo(CommandInfo::Type::SingleCommand));
        } else if(m_enabled) {
            m_enabled = false;
            emit enabledChanged(m_enabled);
        }
    }

    m_dirty = false;
}
