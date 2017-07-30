// #include <QGuiApplication>
#include <QApplication>
#include <QQmlApplicationEngine>
#include <QtQml>
#include <QSurfaceFormat>
#include <QOpenGLContext>
#include <QQuickWindow>
#include <QQuickView>
#include <QmlPreviewer>
#ifdef Q_OS_LINUX
#include <locale>
#endif
#include "mousemover.h"
#include "mysimulator.h"
#include "highlighter.h"
#include "LammpsWrappers/simulatorcontrols/cpcompute.h"
#include "LammpsWrappers/simulatorcontrols/cpfix.h"
#include "LammpsWrappers/simulatorcontrols/cpvariable.h"
#include "LammpsWrappers/simulatorcontrols/nvt.h"
#include "LammpsWrappers/simulatorcontrols/rdf.h"
#include "performance.h"
#include "vendor.h"
#include "datasource.h"
#include "LammpsWrappers/atoms.h"
#include "LammpsWrappers/system.h"
#include "LammpsWrappers/groups.h"
#include "LammpsWrappers/fixes.h"
#include "LammpsWrappers/regions.h"
#include "LammpsWrappers/modifiers/modifiers.h"
#include "LammpsWrappers/computes.h"
#include "LammpsWrappers/variables.h"
#include "LammpsWrappers/units.h"
#include "dataproviders/data1d.h"
#include "dataproviders/data2d.h"
#include "dataproviders/dataprovider.h"
#include "linenumbers.h"
#include "windowgl2.h"
#include "codeeditorbackend.h"
#include "states.h"
#include "parsefileuploader.h"
#include "standardpaths.h"
#include "keysequence.h"
#include <mpi.h>
#include <input.h>

using namespace LAMMPS_NS;
int regularLAMMPS (int argc, char **argv)
{
  MPI_Init(&argc,&argv);

#ifdef LAMMPS_EXCEPTIONS
  try {
    LAMMPS *lammps = new LAMMPS(argc,argv,MPI_COMM_WORLD);
    lammps->input->file();
    delete lammps;
  } catch(LAMMPSAbortException & ae) {
    MPI_Abort(ae.universe, 1);
  } catch(LAMMPSException & e) {
    MPI_Finalize();
    exit(1);
  }
#else
  LAMMPS *lammps = new LAMMPS(argc,argv,MPI_COMM_WORLD);
  lammps->input->file();
  delete lammps;
#endif
  MPI_Barrier(MPI_COMM_WORLD);
  MPI_Finalize();
}

void copyExamplesToLocalFolder()
{
    QString dataDirPath = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
    QDir dataDir(dataDirPath);
    QDir rootQrcFolder(":/");
    QDirIterator exampleIterator(":/examples", QStringList() << "*", QDir::Files, QDirIterator::Subdirectories);
    while(exampleIterator.hasNext()) {
        const QString &qrcFileName = exampleIterator.next();
        QFileInfo qrcFileInfo(qrcFileName);
        QString qrcDirPath = qrcFileInfo.dir().absolutePath();
        QString relativeDirPath = rootQrcFolder.relativeFilePath(qrcDirPath);
        QString relativeFilePath = rootQrcFolder.relativeFilePath(qrcFileName);

        QString targetDirPath = dataDir.absoluteFilePath(relativeDirPath);
        QDir targetDir(targetDirPath);
        if(!targetDir.exists()) {
            bool pathCreated = targetDir.mkpath(".");
            if(!pathCreated) {
                qWarning() << "Could not create" << targetDir.absolutePath();
                continue;
            }
        }

        QString targetPath = dataDir.absoluteFilePath(relativeFilePath);

        if(QFile(targetPath).exists()) {
            continue;
        }

        QFile file(qrcFileName);
        bool fileCopied = file.copy(targetPath);
        if(!fileCopied) {
            qWarning() << "Could not copy" << qrcFileName << "to" << targetPath;
            continue;
        }
        QFile targetFile(targetPath);
        targetFile.setPermissions(targetFile.permissions() | QFile::WriteOwner | QFile::WriteUser);
    }
}

void showDataDir()
{
    QString dir = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
    qDebug() << "Atomify contents are found in " << dir;
    exit(0);
}

int main(int argc, char *argv[])
{
    QVariant scriptToLoad;
    if(argc>1) {
        if(strcmp(argv[1], "--showdatadir")==0) {
            // We need to create Qt app for this, but now we know that the user does not
            // want to run a regular script.
        } else if(strcmp(argv[1], "--version")==0) {
            printf("2.0.10");
            exit(0);
        } else {
            // We might want to run a script with LAMMPS console or Atomify
            bool gFlag = false;  // -g means GUI enabled
            bool inFlag = false; // -in flag means script
            for(int i=0; i<argc; i++) {
                if(strcmp(argv[i], "-g")==0) gFlag = true;
                else if(strcmp(argv[i], "-in")==0) inFlag = true;
                else {
                    // We only support GUI mode with
                    // atomify -in file -g
                    // or other placement of -g. So assuming that anything else is the script.
                    // If filename has space, we fail :P
                    QString fileName = QString::fromUtf8(argv[i]);

                    QString fullPath = QDir(QDir::currentPath()).filePath(fileName);
                    QFileInfo info(fullPath);
                    if(info.exists()) {
                        scriptToLoad = QVariant::fromValue<QString>(fullPath);
                    } else {
                        QFileInfo info(fileName);
                        if(!info.exists()) {
                            qDebug() << "Error, could not find file " << fileName;
                        } else {
                            scriptToLoad = QVariant::fromValue<QString>(fileName);
                        }
                    }
                }
            }

            if(gFlag && inFlag) {
                qDebug() << "Will load script " << scriptToLoad;
            } else {
                return regularLAMMPS(argc, argv);
            }
        }
    }

    qmlRegisterType<AtomifySimulator>("Atomify", 1, 0, "AtomifySimulator");
    qmlRegisterType<CPCompute>("Atomify", 1, 0, "Compute");
    qmlRegisterType<CPFix>("Atomify", 1, 0, "Fix");
    qmlRegisterType<CPVariable>("Atomify", 1, 0, "Variable");
    qmlRegisterType<NVT>("Atomify", 1, 0, "NVT");
    qmlRegisterType<RDF>("Atomify", 1, 0, "RDF");
    qmlRegisterType<DataSource>("Atomify", 1, 0, "DataSource");
    qmlRegisterType<Atoms>("Atomify", 1, 0, "Atoms");
    qmlRegisterType<System>("Atomify", 1, 0, "System");
    qmlRegisterType<Units>("Atomify", 1, 0, "Units");
    qmlRegisterType<Groups>("Atomify", 1, 0, "Groups");
    qmlRegisterType<Computes>("Atomify", 1, 0, "Computes");
    qmlRegisterType<Variables>("Atomify", 1, 0, "Variables");
    qmlRegisterType<Fixes>("Atomify", 1, 0, "Fixes");
    qmlRegisterType<Regions>("Atomify", 1, 0, "Regions");
    qmlRegisterType<DataProvider>("Atomify", 1, 0, "DataProvider");
    qmlRegisterType<Data1D>("Atomify", 1, 0, "Data1D");
    qmlRegisterType<Data2D>("Atomify", 1, 0, "Data2D");

    qmlRegisterType<ColorModifier>("Atomify", 1, 0, "ColorModifier");
    qmlRegisterType<PeriodicImages>("Atomify", 1, 0, "PeriodicImages");
    qmlRegisterType<GroupModifier>("Atomify", 1, 0, "GroupModifier");
    qmlRegisterType<RegionModifier>("Atomify", 1, 0, "RegionModifier");
    qmlRegisterType<PropertyModifier>("Atomify", 1, 0, "PropertyModifier");
    qmlRegisterType<Slice>("Atomify", 1, 0, "SliceModifier");

    qmlRegisterType<LineNumbers>("Atomify", 1, 0, "LineNumbers");
    qmlRegisterType<CodeEditorBackend>("Atomify", 1, 0, "CodeEditorBackend");
    qmlRegisterType<HighlighterWrapper>("Atomify", 1, 0, "Highlighter");
    qmlRegisterType<WindowGL2>("Atomify", 1, 0, "WindowGL2");
    qmlRegisterType<MouseMover>("Atomify", 1, 0, "MouseMover");
    qmlRegisterType<States>("Atomify", 1, 0, "States");
    qmlRegisterType<Performance>("Atomify", 1, 0, "Performance");

    qmlRegisterType<ParseFileUploader>("Atomify", 1, 0, "ParseFileUploader");

    qmlRegisterSingletonType<StandardPaths>("StandardPaths", 1, 0, "StandardPaths", &StandardPaths::qmlInstance);

    qmlRegisterSingletonType<KeySequence>("KeySequence", 1, 0, "KeySequence", &KeySequence::qmlInstance);

    // QGuiApplication app(argc, argv);
    QApplication app(argc, argv);
    app.setOrganizationName("Ovilab");
    app.setOrganizationDomain("ovilab");
    app.setApplicationName("Atomify");
    QSurfaceFormat format;
#ifndef Q_OS_ANDROID
    format.setMajorVersion(3);
    format.setMinorVersion(3);
    format.setProfile(QSurfaceFormat::CoreProfile);
    // format.setSamples(32);
#endif
    QSurfaceFormat::setDefaultFormat(format);

    if(argc>1) {
        if(strcmp(argv[1], "--showdatadir")==0) {
            showDataDir();
        }
    }

    copyExamplesToLocalFolder();

    // Application version
    QQmlApplicationEngine engine;
    QmlPreviewer previewer(app);
    qpm::init(app, engine);
    // if(argc > 2) {
    //     previewer.show();
    // } else {
        engine.load(QUrl(QStringLiteral("qrc:/main.qml")));
        QWindow *window = qobject_cast<QWindow*>(engine.rootObjects()[0]);
        window->setProperty("scriptToLoad", scriptToLoad);
#ifdef Q_OS_MAC
//        window->setIcon(QIcon(":/images/atomify_logo.icns"));
//        app.setWindowIcon(QIcon(":/images/atomify_logo.icns"));
        window->setIcon(QIcon("../Resources/icon.icns"));
        app.setWindowIcon(QIcon("../Resources/icon.icns"));
#endif
#ifdef Q_OS_LINUX
        setlocale(LC_ALL, "C");
        setlocale(LC_NUMERIC, "C");
#endif

        for(QKeySequence k : QKeySequence::keyBindings(QKeySequence::FullScreen)) {
            qDebug() << "Use " << k.toString() << " to toggle fullscreen.";
        }
    // }

    return app.exec();
}
