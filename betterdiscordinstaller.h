#ifndef BETTERDISCORDINSTALLER_H
#define BETTERDISCORDINSTALLER_H

#include <QMainWindow>
#include <QFileDialog>
#include <QSysInfo>
#include <QtNetwork>
#include <qtconcurrentrun.h>
#include <fstream>
#include <unistd.h>
#include <sys/types.h>
#include <pwd.h>
#include <vector>
#include <regex>
#include <typeinfo>
#include <iostream>
#include <json.hpp>
#include <string>
#include <zip.h>
#include <quazip.h>
#include <quazipfile.h>
#include <iterator>

namespace Ui {
class BetterDiscordInstaller;
}

class BetterDiscordInstaller : public QMainWindow
{

    Q_OBJECT
    typedef struct {
        std::string path;
        std::string consPath(std::string dist);
    }InstallDetails;

    using InstallPaths = std::map<std::string, InstallDetails>;
    static InstallPaths installPaths;
    std::fstream archive;
    std::string activeDist;
    std::string os;
    uint64_t baseOffset;

public:
    explicit BetterDiscordInstaller(QWidget *parent = 0);
    ~BetterDiscordInstaller();

    static std::string findLatestVer(std::string base);
    static std::string getHomeDir();
    bool searchDist(QString dist);
    void install(std::string path);
    void extract(std::string path);
    std::string getHeader(std::string path);
    void parseRoot(nlohmann::json data, std::string path);
    bool getBD(QString bdPath);
    void unarchive(QString path);
    void injectBD(std::string path);
    bool dirContainsArch(QString path);

signals:
    void sendInstallInfo(QString str);

private slots:
    void browseFiles();
    void distChecked();
    void pathNotEmpty(QString str);
    void installClicked();
    void installInfo(QString str);
    void licenseClicked();
    void backClicked();

private:
    Ui::BetterDiscordInstaller *ui;
};

#endif // BETTERDISCORDINSTALLER_H
