#ifndef MODIFIER_H
#define MODIFIER_H
#include <QObject>
#include <SimVis/SphereData>
#include <SimVis/CylinderData>
#include "../atomdata.h"

class Modifier : public QObject
{
    Q_OBJECT
    Q_PROPERTY(bool enabled READ enabled WRITE setEnabled NOTIFY enabledChanged)
protected:
    class System *m_system = nullptr;

public:
    Modifier(class System *system);
    virtual void apply(AtomData &atomData) = 0;
    bool enabled() const;

public slots:
    void setEnabled(bool enabled);

signals:
    void enabledChanged(bool enabled);
private:
    bool m_enabled = true;
};

#endif // MODIFIER_H
