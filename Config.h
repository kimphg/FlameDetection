#ifndef CCONFIG_H
#define CCONFIG_H

#include <QApplication>
#include <QFile>
#include <QTextStream>
#include <string>
#include <QRect>
#include "common.h"

#define CONF_PATH           "..//Config/"
#define XML_FILE            "..//Config//config.xml"


struct Config_t
{
    std::string strCamUrl;
    short frmPosX, frmPosY, frmWidth, frmHeight;
};

class CConfig
{
public:
    Config_t _config;
public:
    CConfig();
    void setDefault();
    void SaveXmlFile();
    void LoadXmlFile();
};

#endif // CCONFIG_H
