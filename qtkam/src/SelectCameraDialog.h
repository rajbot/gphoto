#ifndef SELECTCAMERADIALOG_H
#define SELECTCAMERADIALOG_H

#include <kdialog.h>

class QLabel;
class QWidget;
class QComboBox;

class SelectCameraDialog : public KDialog
{
    Q_OBJECT

public:
    SelectCameraDialog(QWidget* parent);

private:
    QLabel *model_label, *port_label, *speed_label;
    QComboBox *model_combo, *port_combo, *speed_combo;

private slots:
    void modelChanged(int i);
    void portChanged(int i);
    void saveSettings();
};

#endif
