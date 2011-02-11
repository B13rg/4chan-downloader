#include "ui4chan.h"
#include "ui_ui4chan.h"

UI4chan::UI4chan(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::UI4chan)
{
    ui->setupUi(this);
    p = new Parser();
    timer = new QTimer();

    iconSize.setHeight(100);
    iconSize.setWidth(100);

    deleteFileAction = new QAction(QString("Delete File"), this);
    deleteFileAction->setIcon(QIcon(":/icons/resources/remove.png"));
    reloadFileAction = new QAction("Reload File", this);
    reloadFileAction->setIcon(QIcon(":/icons/resources/reload.png"));

//    listModel = new QAbstractListModel(ui->listView);

    timeoutValues << 15 << 30 << 60 << 120 << 300;

    foreach (int i, timeoutValues) {
        if (i <= 60)
            ui->comboBox->addItem(QString("every %1 seconds").arg(i));
        else
            ui->comboBox->addItem(QString("every %1 minutes").arg(i/60), i);
    }

    ui->progressBar->setMinimum(0);
    ui->progressBar->setMaximum(0);
    ui->progressBar->setValue(0);
    ui->progressBar->setEnabled(false);

    ui->listWidget->setIconSize(iconSize);
    ui->listWidget->setGridSize(QSize(iconSize.width()+10,iconSize.height()+10));

    connect(p, SIGNAL(downloadedCountChanged(int)), ui->progressBar, SLOT(setValue(int)));
    connect(p, SIGNAL(totalCountChanged(int)), ui->progressBar, SLOT(setMaximum(int)));
    connect(p, SIGNAL(finished()), this, SLOT(downloadsFinished()));
    connect(p, SIGNAL(fileFinished(QString)), this, SLOT(addThumbnail(QString)));
    connect(p, SIGNAL(error(int)), this, SLOT(errorHandler(int)));
    connect(p, SIGNAL(threadTitleChanged(QString)), ui->lTitle, SLOT(setText(QString)));

    connect(deleteFileAction, SIGNAL(triggered()), this, SLOT(deleteFile()));
    connect(reloadFileAction, SIGNAL(triggered()), this, SLOT(reloadFile()));

    connect(timer, SIGNAL(timeout()), this, SLOT(triggerRescan()));
}

UI4chan::~UI4chan()
{
    delete ui;
}

void UI4chan::start(void) {
    QDir dir;

    ui->leURI->setEnabled(false);
    p->setURI(ui->leURI->text());
    dir.setPath(ui->leSavepath->text());
    if (dir.exists()) {
        ui->leSavepath->setEnabled(false);
        p->setSavePath(ui->leSavepath->text());
        p->start();

        ui->btnStart->setEnabled(false);
        ui->btnStop->setEnabled(true);
        ui->cbRescan->setEnabled(false);
        ui->comboBox->setEnabled(false);
        ui->progressBar->setEnabled(true);

        if (ui->cbRescan->isChecked()) {
//            qDebug() << "Timer interval: " << timeoutValues.at(ui->comboBox->currentIndex())*1000;
            timer->setInterval(timeoutValues.at(ui->comboBox->currentIndex())*1000);

            timer->start();
        }
    }
    else
    {
//        qDebug() << "Directory does not exist";
    }
}

void UI4chan::stop(void) {
    timer->stop();
    ui->btnStart->setEnabled(true);
    ui->btnStop->setEnabled(false);
    ui->leURI->setEnabled(true);
    ui->leSavepath->setEnabled(true);
    ui->cbRescan->setEnabled(true);
    ui->comboBox->setEnabled(true);
    ui->progressBar->setEnabled(false);
}

void UI4chan::chooseLocation(void) {
    QString loc;

    loc = QFileDialog::getExistingDirectory(this, "Choose storage directory", ui->leSavepath->text());

    ui->leSavepath->setText(loc);
}

void UI4chan::triggerRescan(void) {
//    qDebug() << "Triggering rescan";

    p->start();

    timer->start();
}

void UI4chan::addThumbnail(QString filename) {
    QListWidgetItem* item;
    QPixmap pixmapLarge, pixmapSmall;

    pixmapLarge.load(filename);
    pixmapSmall = pixmapLarge.scaled(iconSize,Qt::KeepAspectRatio);

    item = new QListWidgetItem(
            QIcon(pixmapSmall),
            filename,
            ui->listWidget);

    ui->listWidget->addItem(item);


}

void UI4chan::downloadsFinished() {
    if (!ui->cbRescan->isChecked())
        stop();
}

void UI4chan::on_listWidget_customContextMenuRequested(QPoint pos)
{
    QMenu contextMenu(tr("Context menu"), this);

    contextMenu.addAction(deleteFileAction);
    contextMenu.addAction(reloadFileAction);
    contextMenu.exec(ui->listWidget->mapTo(this,mapToGlobal(pos)));
}

void UI4chan::deleteFile(void) {
    QFile f;
    QString filename;

    filename = ui->listWidget->currentItem()->text();
    if (filename != "") {
//        qDebug() << "Going to delete " << filename;
        f.setFileName(filename);

        if (f.exists()) {
            f.remove();

            ui->listWidget->takeItem(ui->listWidget->currentRow());
        }
    }
}

void UI4chan::reloadFile(void) {
    QString filename;
    QFile f;

    filename = ui->listWidget->currentItem()->text();
    if (filename != "") {
//        qDebug() << "Going to reload " << filename;

        f.setFileName(filename);

        if (f.exists()) {
            f.remove();

            ui->listWidget->takeItem(ui->listWidget->currentRow());

            p->reloadFile(filename);
        }
    }
}

void UI4chan::errorHandler(int err) {
    switch (err) {
    case 404:
        p->stop();
        stop();

        emit errorMessage("404 - Page not found");

        break;

    default:
        break;
    }
}
