#include <qwidget.h>
#include <qlabel.h>
#include <qlayout.h>
#include <qcombobox.h>
#include <qpushbutton.h>
#include <qstringlist.h>
#include <kdialog.h>
#include <kmessagebox.h>
#include <klocale.h>

#include "SelectCameraDialog.h"
#include "SelectCameraDialog.moc"
#include "GPInterface.h"

SelectCameraDialog::SelectCameraDialog(QWidget* parent=0) : 
    KDialog(parent,"selectcamera",true)
{
    int selected;

    /* Set title */
    setCaption("Select Camera");

    /* Create new layout */
    QVBoxLayout* layout = new QVBoxLayout(this,5);
    layout->setMargin(10);

    /* Draw Camera Model stuff */
    model_label = new QLabel(this);
    model_label->setText(i18n("Select Camera Model:"));
    model_combo = new QComboBox(this);
    connect(model_combo,SIGNAL(activated(int)),this,SLOT(modelChanged(int)));
    layout->addWidget(model_label);
    layout->addWidget(model_combo); 
     
    /* Draw Camera Port stuff */
    port_label = new QLabel(this);
    port_label->setText(i18n("Select Port:"));
    port_combo = new QComboBox(this);
    connect(port_combo,SIGNAL(activated(int)), this,SLOT(portChanged(int)));
    layout->addWidget(port_label);
    layout->addWidget(port_combo);
    
    /* Draw Port Speed stuff */
    speed_label = new QLabel(this);
    speed_label->setText(i18n("Select Speed:"));
    speed_combo = new QComboBox(this);
    //connect(speed_combo,SIGNAL(activated(int)),this,SLOT(speed_changed(int)));
    layout->addWidget(speed_label);
    layout->addWidget(speed_combo);

    /* Draw Ok & Cancel Buttons */
    QPushButton *ok = new QPushButton(i18n("Ok"),this);
    connect(ok,SIGNAL(clicked()), this, SLOT(saveSettings()));
    QPushButton *cancel = new QPushButton(i18n("Cancel"),this);
    connect(cancel, SIGNAL(clicked()), this, SLOT(reject()));
    
    QHBoxLayout *l = new QHBoxLayout(layout);
    l->setMargin(10);
    l->addWidget(ok);
    l->addWidget(cancel);

    /* Populate camera list */
    QStringList camlist = GPInterface::getSupportedCameras();
    model_combo->insertStringList(camlist);
    
    /* Select the current camera from list */
    selected = camlist.findIndex(GPInterface::getCamera());
    if (selected >= 0) {
        model_combo->setCurrentItem(selected);
        modelChanged(selected);
    }
    else
        modelChanged(0);
}


/* 
 * Activated when a new camera model is selected.
 * Populates the speed and the port list.
 */
void SelectCameraDialog::modelChanged(int)
{
    int selected;

    /* Get supported ports for the selected camera */
    QStringList portlist = GPInterface::getSupportedPorts(
        model_combo->currentText());
    port_combo->clear();
    port_combo->insertStringList(portlist);

    /* Select the current port from list */
    selected = portlist.findIndex(GPInterface::getPort());
    if (selected >= 0)
        port_combo->setCurrentItem(selected);
        
    /* Get supported speeds for the selected camera */
    QStringList speedlist = GPInterface::getSupportedSpeeds(
        model_combo->currentText());
    speed_combo->clear();
    speed_combo->insertStringList(speedlist);

    /* Disable combo if empty */
    port_combo->setEnabled(port_combo->count() > 0);
    
    /* Notify that the port might have changed */
    portChanged(selected);
}

void SelectCameraDialog::portChanged(int i)
{
    /* Retrieve selected port. */
    /* FIXME: Comparing strings is not clean. Should associate types, 
       to combo box entries, but not so obvious how with QComboBoxes. */
    speed_combo->setEnabled((port_combo->text(i) == "serial:"));
}

void SelectCameraDialog::saveSettings()
{
    if (model_combo->currentText().isNull()) {
        KMessageBox::error(this, i18n("You must select a camera"));
        return;
    }
    else {
        if (port_combo->isEnabled() && port_combo->currentText().isNull()) {
            KMessageBox::error(this, i18n("You must select a port"));
            return;
        }
        
        /* Save the settings */
        GPInterface::setCamera(model_combo->currentText());
        GPInterface::setPort(port_combo->currentText());
        GPInterface::setSpeed(speed_combo->currentText());
    }

    /* Close the dialog and notify we changed the camera */
    accept();
}
