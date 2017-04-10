#include "Config.h"
#include "qdir.h"

CConfig::CConfig()
{    
    LoadXmlFile();
}

void CConfig::LoadXmlFile()
{
    if (!QDir(CONF_PATH).exists())
        QDir().mkdir(CONF_PATH);

    CvFileStorage* fs = NULL;

    try
    {
        fs = cvOpenFileStorage(XML_FILE, 0, CV_STORAGE_READ);
    }
    catch (...)
    {
        setDefault();
        return;
    }


    if (fs == NULL)
    {
        setDefault();
        return;
    }



    cvReleaseFileStorage(&fs);
}

void CConfig::setDefault()
{

    SaveXmlFile();    
}



void CConfig::SaveXmlFile()
{
    CvFileStorage* fs = cvOpenFileStorage(XML_FILE, 0, CV_STORAGE_WRITE);

    cvReleaseFileStorage(&fs);
}
