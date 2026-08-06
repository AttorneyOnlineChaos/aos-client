#include "hardware_functions.h"

#include <QDebug>
#include <QtGlobal>

#include <stdio.h>

#if (defined(_WIN32) || defined(_WIN64))
// clang-format off
#include <windows.h>
#include <sddl.h>
// clang-format on

QString spritechat::get_hdid()
{
  HANDLE hToken;
  HANDLE pHandle;
  PTOKEN_USER pToken;
  DWORD uSize = 0;
  LPWSTR HDIDParam;

  pHandle = GetCurrentProcess();
  OpenProcessToken(pHandle, TOKEN_QUERY, &hToken);
  if (!GetTokenInformation(hToken, (TOKEN_INFORMATION_CLASS)1, NULL, 0, &uSize))
  {
    if (GetLastError() != ERROR_INSUFFICIENT_BUFFER)
    {
      CloseHandle(hToken);
      return "gxsps32sa9fnwic92mfbs1";
    }
  }

  pToken = (PTOKEN_USER)GlobalAlloc(GPTR, uSize);

  if (!GetTokenInformation(hToken, (TOKEN_INFORMATION_CLASS)1, pToken, uSize, &uSize))
  {
    CloseHandle(hToken);
    return "gxsps32sa9fnwic92mfbs2";
  }

  ConvertSidToStringSidW(pToken->User.Sid, &HDIDParam);
  QString returnHDID = QString::fromWCharArray(HDIDParam);
  CloseHandle(hToken);
  return returnHDID;
}
#else
#include <QSysInfo>

QByteArray machineId;

QString spritechat::get_hdid()
{
  machineId = QSysInfo::machineUniqueId();
  return QString(machineId);
}

#endif
