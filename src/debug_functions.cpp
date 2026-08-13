#include "debug_functions.h"

#include <QCoreApplication>
#include <QDialogButtonBox>
#include <QElapsedTimer>
#include <QEventLoop>
#include <QPushButton>
#include <QTimer>

#include <functional>

namespace
{
void call_message(QMessageBox::Icon icon, const QString &p_message)
{
  QString title;
  switch (icon)
  {
  default:
    title = QCoreApplication::translate("debug_functions", "Notice");
    break;

  case QMessageBox::Warning:
    title = QCoreApplication::translate("debug_functions", "Warning");
    break;

  case QMessageBox::Critical:
    title = QCoreApplication::translate("debug_functions", "Error");
    break;
  }

  auto *msgBox = new QMessageBox;

  msgBox->setAttribute(Qt::WA_DeleteOnClose);
  msgBox->setIcon(icon);
  msgBox->setText(p_message);
  msgBox->setWindowTitle(title);

  msgBox->setStandardButtons(QMessageBox::Ok);
  msgBox->setDefaultButton(QMessageBox::Ok);
  msgBox->defaultButton()->setEnabled(false);

  QTimer intervalTimer;
  intervalTimer.setInterval(1000);

  int counter = 3;
  const auto updateCounter = [msgBox, &counter] {
    if (counter <= 0)
    {
      return;
    }
    msgBox->defaultButton()->setText(QString("%1 (%2)").arg(QDialogButtonBox::tr("OK")).arg(counter));
    counter--;
  };

  QObject::connect(&intervalTimer, &QTimer::timeout, msgBox, updateCounter);
  intervalTimer.start();
  updateCounter();

  QTimer::singleShot(3000, msgBox, [msgBox, &intervalTimer] {
    msgBox->defaultButton()->setEnabled(true);
    msgBox->defaultButton()->setText(QDialogButtonBox::tr("OK"));
    intervalTimer.stop();
  });

  QEventLoop loop;
  QObject::connect(msgBox, &QDialog::finished, &loop, &QEventLoop::quit);
  QObject::connect(msgBox, &QObject::destroyed, &loop, &QEventLoop::quit);

  msgBox->setWindowModality(Qt::ApplicationModal);
  msgBox->show();

  loop.exec(QEventLoop::ExcludeSocketNotifiers);
}
} // namespace

void spritechat::call_notice(const QString &p_message)
{
  call_message(QMessageBox::Information, p_message);
}

void spritechat::call_warning(const QString &p_message)
{
  call_message(QMessageBox::Warning, QCoreApplication::translate("debug_functions", "Warning: %1").arg(p_message));
}

void spritechat::call_error(const QString &p_message)
{
  call_message(QMessageBox::Critical, QCoreApplication::translate("debug_functions", "Error: %1").arg(p_message));
}
