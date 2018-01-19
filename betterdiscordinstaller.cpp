#include "betterdiscordinstaller.h"
#include "ui_betterdiscordinstaller.h"

std::string BetterDiscordInstaller::getHomeDir(){
    struct passwd *pw = getpwuid(getuid());
    return std::string(pw->pw_dir) + "/";
}

std::string BetterDiscordInstaller::InstallDetails::consPath(std::string dist) {
    std::string path = getHomeDir() + this->path + dist;
    path = findLatestVer(path);
    return path;
}

BetterDiscordInstaller::InstallPaths BetterDiscordInstaller::installPaths = []
{
    InstallPaths paths;
    paths = {{"darwin", {
                  {"Library/Application Support/"}
             }},
              {"linux", {
                  {".config/"}
              }}};

    return paths;
}();

bool BetterDiscordInstaller::searchDist(QString dist){
    std::string str = dist.toUtf8().constData();
    return installPaths[os].consPath(str) != "";
}

BetterDiscordInstaller::BetterDiscordInstaller(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::BetterDiscordInstaller)
{
    ui->setupUi(this);

   os = QSysInfo::kernelType().toUtf8().constData();

    QRadioButton *ckBoxes[] = {ui->discordcanary, ui->discordptb, ui->discord};
    for(int i = 0; i < 3; i++) {
        connect(ckBoxes[i], SIGNAL(clicked(bool)), this, SLOT(distChecked()));
        QString name = ckBoxes[i]->objectName();
        QString lName = QString("found" + name);
        QLabel *label = this->findChild<QLabel*>(lName, Qt::FindChildrenRecursively);
        if(searchDist(name)) {
            label->setText("Found");
            label->setStyleSheet("QLabel {color: green;}");
        }else {
            label->setText("Not found");
            label->setStyleSheet("QLabel {color: red;}");
        }
    }
    connect(ui->browse, SIGNAL(clicked()), this, SLOT(browseFiles()));
    connect(ui->pathEdit, SIGNAL(textChanged(QString)), this, SLOT(pathNotEmpty(QString)));
    connect(ui->install, SIGNAL(clicked(bool)), this, SLOT(installClicked()));
    connect(ui->quit, SIGNAL(clicked(bool)), this, SLOT(close()));
    connect(ui->license, SIGNAL(clicked(bool)), this, SLOT(licenseClicked()));
    connect(ui->back, SIGNAL(clicked(bool)), this, SLOT(backClicked()));
}

BetterDiscordInstaller::~BetterDiscordInstaller()
{
    delete ui;
}

void BetterDiscordInstaller::installInfo(QString str) {
    ui->feed->insertPlainText("\n" + str);
}

void BetterDiscordInstaller::licenseClicked() {
    ui->stackedWidget->setCurrentIndex(1);
}

void BetterDiscordInstaller::backClicked(){
    ui->stackedWidget->setCurrentIndex(0);
}

void BetterDiscordInstaller::browseFiles() {
    QString path = QString::fromStdString(getHomeDir() +  installPaths[os].path);
    QString dir = QFileDialog::getExistingDirectory(this,tr("Open Directory"),
                                                    path,
                                                    QFileDialog::ShowDirsOnly);
    ui->pathEdit->setText(dir + "/");
}

void BetterDiscordInstaller::distChecked() {
    ui->pathEdit->clear();
    activeDist = sender()->objectName().toUtf8().constData();
    ui->pathEdit->setText(QString::fromStdString(installPaths[os].consPath(activeDist)));
}

bool BetterDiscordInstaller::dirContainsArch(QString path) {
    QDir dir(path);
    QStringList list = dir.entryList(QStringList() << "*.asar", QDir::Files);
    return list.size() > 0;
}

void BetterDiscordInstaller::pathNotEmpty(QString str) {
    if (str != "") {
        ui->install->setEnabled(true);
        if(dirContainsArch(str)){
            ui->containsLabel->setStyleSheet("QLabel {color : green}");
        }else {
            ui->containsLabel->setStyleSheet("QLabel {color : red}");
        }
    }else {
        ui->containsLabel->setStyleSheet("QLabel {color : black}");
        ui->install->setEnabled(false);
    }
}

void BetterDiscordInstaller::installClicked(){
    ui->install->setDisabled(true);
    install(ui->pathEdit->text().toUtf8().constData());
}

std::string BetterDiscordInstaller::findLatestVer(std::string str) {
    std::string fullPath = "";
    std::vector<std::string> paths;

    QDirIterator it(QString::fromStdString(str));
    while (it.hasNext()){
        std::string path = it.filePath().toUtf8().constData();
        if (std::regex_match (path, std::regex(".*([^\\/]|[0-9.])+\\d$") )){
            fullPath = path;
            paths.push_back(fullPath);
        }
        it.next();
    }
    return paths.size() == 0 ? "" : fullPath + "/modules/discord_desktop_core/";
}

void BetterDiscordInstaller::install(std::string path) {
    ui->feed->insertPlainText("Starting installation");

//    std::cout << "starting installation" << std::endl;
//    connect(this, SIGNAL(sendInstallInfo(QString)), this, SLOT(installInfo(QString)));
//    QFuture<void> future = QtConcurrent::run(this, &BetterDiscordInstaller::extract, path);

//    while(!future.isFinished()) {
//        qApp->processEvents();
//    }
    extract(path);

    QString bdPath = QString::fromStdString(path+"node_modules/");

    ui->feed->insertPlainText("\nDownloading BetterDiscord from Zere's fork");
    if(!getBD(bdPath)){
        return;
    };
    unarchive(bdPath);

    injectBD(path);

//    disconnect(this, SIGNAL(sendInstallInfo(QString)), 0, 0);

    ui->feed->insertPlainText("\nDone\n\n");
    ui->install->setDisabled(false);
}

void BetterDiscordInstaller::extract(std::string path) {
//    emit sendInstallInfo("Extracting core.asar");
    ui->feed->insertPlainText("\nExtracting core.asar");
    std::cout << path << std::endl;
    std::string header = getHeader(path);
    auto j = nlohmann::json::parse(header);
    parseRoot(j, path);
    archive.close();
    baseOffset = 0;
//    emit sendInstallInfo("Done extracting");
    ui->feed->insertPlainText("\nDone extracting");
}

std::string BetterDiscordInstaller::getHeader(std::string path) {
    archive.open(path + "core.asar", std::ios::in);
    if (!archive) {
//        emit sendInstallInfo("Failed to open archive");
        ui->feed->insertPlainText("\nFialed to open archive " + QString::fromStdString(path) + "core.asar");
        return NULL;
    }

    uint32_t buff[16];
    archive.read((char*)&buff[0], 16);
    uint32_t headerStringSize = buff[3];

    baseOffset = 16 + uint64_t(headerStringSize);
    baseOffset += baseOffset % 4;
    std::vector<unsigned char>headerBuff(headerStringSize);
    archive.read((char*)&headerBuff[0], uint64_t(headerStringSize));
    std::string header(headerBuff.begin(), headerBuff.end());

    return header;
}

void BetterDiscordInstaller::parseRoot(nlohmann::json data, std::string path) {
    qApp->processEvents();
    for (nlohmann::json::iterator it = data.begin(); it != data.end(); it++) {
        if(it.key() == "files" || it.key() == "bin"){
            parseRoot(it.value(), path);
            continue;
        }

        if(it.value()["files"].is_object()) {
            std::string newPath = path + "/" + it.key();
            QDir().mkpath(QString::fromStdString(newPath));
            parseRoot(it.value(), newPath);
        } else {
            int64_t size = 0;
            if (!(it.value()["size"].is_null())){
                size = it.value()["size"];
            }

            if(!(it.value()["offset"].is_null())) {
                std::string offset = it.value()["offset"];
                int64_t offset64 = std::strtoll(offset.c_str(), NULL, 10);
                offset64 += baseOffset;

                std::ofstream out(path+"/"+it.key());
                std::vector<char> buff(size);
                archive.seekg(offset64);
                archive.read(&buff[0], size);

                out.write(&buff[0], size);
                out.close();

            }
        }
    }
}

bool BetterDiscordInstaller::getBD(QString bdPath){
    QUrl url ("https://codeload.github.com/rauenzi/BetterDiscordApp/zip/stable16");
    ui->feed->insertPlainText("\n"+ url.toString());
    QNetworkRequest request(url);
    QNetworkAccessManager manager;
    QNetworkReply *reply =  manager.get(request);

    QTimer timer;
    timer.start(300000);
    QEventLoop loop;
    connect(&manager, SIGNAL(finished(QNetworkReply*)), &loop, SLOT(quit()));
    loop.exec();

    if(!(timer.isActive())){
        ui->feed->insertPlainText("\nDownload timed out");
        return false;
    }


    if(reply->error()) {
        ui->feed->insertPlainText(
                    QString("\nFailed to download BetterDiscord: %1").
                    arg(qPrintable(reply->errorString())));
    } else {
        ui->feed->insertPlainText("\nSuccessfully downloaded BetterDiscord");

        QFile file(bdPath+"BD.zip");
        if(!file.open(QIODevice::WriteOnly)){
            ui->feed->insertPlainText(
                        QString("\nFailed to open file for writing: %1").
                        arg(qPrintable(file.errorString())));
        } else {
            file.write(reply->readAll());
            file.close();
            return true;
        }
    }
    return false;
}

void BetterDiscordInstaller::unarchive(QString path) {
    QuaZip zipFile(path+"BD.zip");
    ui->feed->insertPlainText(QString("\nExtracting %1BD.zip").arg(path));
    zipFile.open(QuaZip::mdUnzip);


    for(bool f=zipFile.goToFirstFile(); f; f=zipFile.goToNextFile()) {
        QString filePath = zipFile.getCurrentFileName();

        QuaZipFile zFile(zipFile.getZipName(), filePath);

        std::smatch sm;
        std::string s (filePath.toUtf8().constData());
        s = std::regex_replace (s, std::regex("(^.*?\\/)"), "BetterDiscord/");
        std::regex_match (s, sm, std::regex("(.*\\/)(.*)"));

        QString dir = path + QString::fromStdString(sm[1]);
        filePath = path + QString::fromStdString(sm[0]);

        zFile.open(QIODevice::ReadOnly);
        QByteArray ba = zFile.readAll();
        zFile.close();

        QDir().mkpath(dir);

        if(sm[2] != "") {
            QFile dstFile( filePath );

            dstFile.open( QIODevice::WriteOnly | QIODevice::Text );
            dstFile.write( ba.data() );
            dstFile.close();
        }
    }
    zipFile.close();
    ui->feed->insertPlainText("\nRemoving temporary files");
    QFile::remove(path + "BD.zip");
}

void BetterDiscordInstaller::injectBD(std::string path) {
    ui->feed->insertPlainText(QString("\nInjecting BetterDiscord into %1app/mainScreen.js").arg(QString::fromStdString(path)));
    std::fstream iFile(path + "app/mainScreen.js", std::ios::in);
    std::string line;
    std::vector<std::string> stream;

    while(std::getline(iFile, line)) {
        stream.push_back(line);
        if(line.find("var _url = require('url');") != std::string::npos){
            stream.push_back("var _betterDiscord = require('betterdiscord');");
            stream.push_back("var _betterDiscord2;");
        }
        if(line.find("mainWindow = new _electron.BrowserWindow(mainWindowOptions);") != std::string::npos){
            stream.push_back("_betterDiscord2 = new _betterDiscord.BetterDiscord(mainWindow, false);");
        }
    }

    iFile.close();

    iFile.open(path + "app/mainScreen.js", std::ios::out | std::fstream::trunc);

    std::ostream_iterator<std::string> it(iFile, "\n");
    std::copy(stream.begin(), stream.end(), it);

    iFile.close();
}



