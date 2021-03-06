// albert - a simple application launcher for linux
// Copyright (C) 2014-2017 Manuel Schneider
//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program.  If not, see <http://www.gnu.org/licenses/>.

#include <QFileDialog>
#include <QStandardPaths>
#include <QStringListModel>
#include "configwidget.h"
#include "main.h"
#include "mimetypedialog.h"

/** ***************************************************************************/
Files::ConfigWidget::ConfigWidget(Extension *_extension, QWidget *_parent)
    : QWidget(_parent), extension(_extension) {
    ui.setupUi(this);


    // Paths
    QStringListModel *pathsModel = new QStringListModel(this);
    pathsModel->setStringList(extension->paths());
    ui.listView_paths->setModel(pathsModel);

    connect(pathsModel, &QStringListModel::dataChanged,
            [=](){ extension->setPaths(pathsModel->stringList()); });

    connect(pathsModel, &QStringListModel::rowsInserted,
            [=](){ extension->setPaths(pathsModel->stringList()); });

    connect(pathsModel, &QStringListModel::rowsRemoved,
            [=](){ extension->setPaths(pathsModel->stringList()); });

    // Buttons
    connect(ui.pushButton_add, &QPushButton::clicked, [=](){
        QString path = QFileDialog::getExistingDirectory(
                    this,
                    tr("Choose path"),
                    QStandardPaths::writableLocation(QStandardPaths::HomeLocation));

        if(path.isEmpty())
            return;

        if ( pathsModel->match(pathsModel->index(0, 0), Qt::DisplayRole,
                               QVariant::fromValue(path)).isEmpty() ) {
            pathsModel->insertRow(pathsModel->rowCount());
            pathsModel->setData(pathsModel->index(pathsModel->rowCount()-1,0), path);
        }
    });

    connect(ui.pushButton_remove, &QPushButton::clicked, [=](){
        if ( !ui.listView_paths->currentIndex().isValid() )
            return;
        pathsModel->removeRow(ui.listView_paths->currentIndex().row());
    });

    connect(ui.pushButton_update, &QPushButton::clicked,
            extension, &Extension::updateIndex);

    connect(ui.pushButton_restore, &QPushButton::clicked,
            extension, &Extension::restorePaths);

    /*
     * Initialize the indexing options
     */

    ui.checkBox_hidden->setChecked(extension->indexHidden());
    connect(ui.checkBox_hidden, &QCheckBox::toggled, extension, &Extension::setIndexHidden);

    ui.checkBox_followSymlinks->setChecked(extension->followSymlinks());
    connect(ui.checkBox_followSymlinks, &QCheckBox::toggled, extension, &Extension::setFollowSymlinks);

    ui.checkBox_fuzzy->setChecked(extension->fuzzy());
    connect(ui.checkBox_fuzzy, &QCheckBox::toggled, extension, &Extension::setFuzzy);

    ui.spinBox_interval->setValue(static_cast<int>(extension->scanInterval()));
    connect(ui.spinBox_interval, static_cast<void(QSpinBox::*)(int)>(&QSpinBox::valueChanged),
            extension, &Extension::setScanInterval);

    /*
     * Initialize the mime options
     */

    if (extension->filters().contains("audio/*"))
        ui.checkBox_audio->setCheckState(Qt::Checked);
    else if (std::any_of(extension->filters().begin(), extension->filters().end(),
                        [](const QString & str){ return str.startsWith("audio/"); }))
        ui.checkBox_audio->setCheckState(Qt::PartiallyChecked);
    else
        ui.checkBox_audio->setCheckState(Qt::Unchecked);

    if (extension->filters().contains("video/*"))
        ui.checkBox_video->setCheckState(Qt::Checked);
    else if (std::any_of(extension->filters().begin(), extension->filters().end(),
                        [](const QString & str){ return str.startsWith("video/"); }))
        ui.checkBox_video->setCheckState(Qt::PartiallyChecked);
    else
        ui.checkBox_video->setCheckState(Qt::Unchecked);

    if (extension->filters().contains("image/*"))
        ui.checkBox_image->setCheckState(Qt::Checked);
    else if (std::any_of(extension->filters().begin(), extension->filters().end(),
                         [](const QString & str){ return str.startsWith("image/"); }))
        ui.checkBox_image->setCheckState(Qt::PartiallyChecked);
    else
        ui.checkBox_image->setCheckState(Qt::Unchecked);

    if (extension->filters().contains("application/*"))
        ui.checkBox_docs->setCheckState(Qt::Checked);
    else if (std::any_of(extension->filters().begin(), extension->filters().end(),
                         [](const QString & str){ return str.startsWith("application/"); }))
        ui.checkBox_docs->setCheckState(Qt::PartiallyChecked);
    else
        ui.checkBox_docs->setCheckState(Qt::Unchecked);

    if (extension->filters().contains("inode/directory"))
        ui.checkBox_dirs->setCheckState(Qt::Checked);
    else
        ui.checkBox_dirs->setCheckState(Qt::Unchecked);

    /*
     * Set the actions for checking the mime options
     */

    connect(ui.checkBox_audio, &QCheckBox::clicked, extension, [this]() {
        ui.checkBox_audio->setTristate(false);
        QStringList filters = extension->filters();
        filters.erase(std::remove_if(filters.begin(), filters.end(),
                                     [](const QString &str){ return str.startsWith("audio/"); }),
                      filters.end());
        if (ui.checkBox_audio->checkState() == Qt::Checked)
            filters.push_back("audio/*");
        extension->setFilters(filters);
    });

    connect(ui.checkBox_video, &QCheckBox::clicked, extension, [this]() {
        ui.checkBox_video->setTristate(false);
        QStringList filters = extension->filters();
        filters.erase(std::remove_if(filters.begin(), filters.end(),
                                     [](const QString &str){ return str.startsWith("video/"); }),
                      filters.end());
        if (ui.checkBox_video->checkState() == Qt::Checked)
            filters.push_back("video/*");
        extension->setFilters(filters);
    });

    connect(ui.checkBox_image, &QCheckBox::clicked, extension, [this]() {
        ui.checkBox_image->setTristate(false);
        QStringList filters = extension->filters();
        filters.erase(std::remove_if(filters.begin(), filters.end(),
                                     [](const QString &str){ return str.startsWith("image/"); }),
                      filters.end());
        if (ui.checkBox_image->checkState() == Qt::Checked)
            filters.push_back("image/*");
        extension->setFilters(filters);
    });

    connect(ui.checkBox_docs, &QCheckBox::clicked, extension, [this]() {
        ui.checkBox_docs->setTristate(false);
        QStringList filters = extension->filters();
        filters.erase(std::remove_if(filters.begin(), filters.end(),
                                     [](const QString &str){ return str.startsWith("application/"); }),
                      filters.end());
        if (ui.checkBox_docs->checkState() == Qt::Checked)
            filters.push_back("application/*");
        extension->setFilters(filters);
    });

    connect(ui.checkBox_dirs, &QCheckBox::toggled, extension, [=](bool checked){
        QStringList filters = extension->filters();
        filters.removeAll("inode/directory");
        if (checked)
            filters.push_back("inode/directory");
        extension->setFilters(filters);
    });

    // The advanced button action
    connect(ui.pushButton_advanced, &QPushButton::clicked, [=](){

        MimeTypeDialog dialog(extension->filters(), this);
        dialog.setWindowModality(Qt::WindowModal);
        if ( dialog.exec() ) {

            // If the dialog has been accepted, update extension and checkboxes
            extension->setFilters(dialog.filters());

            // Set the shortcuts
            if (extension->filters().contains("audio/*"))
                ui.checkBox_audio->setCheckState(Qt::Checked);
            else if (std::any_of(extension->filters().begin(), extension->filters().end(),
                                 [](const QString & str){ return str.startsWith("audio/"); }))
                ui.checkBox_audio->setCheckState(Qt::PartiallyChecked);
            else
                ui.checkBox_audio->setCheckState(Qt::Unchecked);

            if (extension->filters().contains("video/*"))
                ui.checkBox_video->setCheckState(Qt::Checked);
            else if (std::any_of(extension->filters().begin(), extension->filters().end(),
                                 [](const QString & str){ return str.startsWith("video/"); }))
                ui.checkBox_video->setCheckState(Qt::PartiallyChecked);
            else
                ui.checkBox_video->setCheckState(Qt::Unchecked);

            if (extension->filters().contains("image/*"))
                ui.checkBox_image->setCheckState(Qt::Checked);
            else if (std::any_of(extension->filters().begin(), extension->filters().end(),
                                 [](const QString & str){ return str.startsWith("image/"); }))
                ui.checkBox_image->setCheckState(Qt::PartiallyChecked);
            else
                ui.checkBox_image->setCheckState(Qt::Unchecked);

            if (extension->filters().contains("application/*"))
                ui.checkBox_docs->setCheckState(Qt::Checked);
            else if (std::any_of(extension->filters().begin(), extension->filters().end(),
                                 [](const QString & str){ return str.startsWith("application/"); }))
                ui.checkBox_docs->setCheckState(Qt::PartiallyChecked);
            else
                ui.checkBox_docs->setCheckState(Qt::Unchecked);

            if (extension->filters().contains("inode/directory"))
                ui.checkBox_dirs->setCheckState(Qt::Checked);
            else
                ui.checkBox_dirs->setCheckState(Qt::Unchecked);
        }
    });

    // Status bar
    connect(extension, &Extension::statusInfo, ui.label_statusbar, &QLabel::setText);
}



/** ***************************************************************************/
Files::ConfigWidget::~ConfigWidget() {

}
