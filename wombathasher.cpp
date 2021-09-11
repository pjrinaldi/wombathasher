#include <stdint.h>
#include <stdio.h>
#include <unistd.h>
#include <errno.h>
#include <stdio.h>
#include <time.h>
#include <string.h>
#include <stdlib.h>

#include <iostream>
#include <QFile>
#include <QDataStream>
#include <QTextStream>
#include <QCommandLineParser>
#include <QDateTime>
#include <QDebug>
#include <QtEndian>
#include <QtConcurrent>
#include "blake3.h"
//#include "lmdb.h"

QFile hashfile;
//int rc;
//MDB_env* lenv;

void DirectoryRecurse(QString dirname, QStringList* filelist, bool isrelpath)
{
    QDir curdir(dirname);
    if(curdir.isEmpty())
	qInfo() << "dir" << dirname << "is empty and will be skipped.";
    else
    {
	QFileInfoList infolist = curdir.entryInfoList(QDir::NoDotAndDotDot | QDir::Files | QDir::Dirs);
	for(int i=0; i < infolist.count(); i++)
	{
	    QFileInfo tmpinfo = infolist.at(i);
            if(tmpinfo.isDir())
                DirectoryRecurse(tmpinfo.absolutePath(), filelist, isrelpath);
            else if(tmpinfo.isFile())
            {
                if(isrelpath)
                    filelist->append(tmpinfo.filePath());
                else
                    filelist->append(tmpinfo.absoluteFilePath());
            }
	}
    }
}

QString HashFiles(QString filename)
{
    QFile bfile(filename);
    if(!bfile.isOpen())
        bfile.open(QIODevice::ReadOnly);
    bfile.seek(0);
    blake3_hasher hasher;
    blake3_hasher_init(&hasher);
    while(!bfile.atEnd())
    {
        QByteArray tmparray = bfile.read(65536);
        blake3_hasher_update(&hasher, tmparray.data(), tmparray.count());
    }
    uint8_t output[BLAKE3_OUT_LEN];
    blake3_hasher_finalize(&hasher, output, BLAKE3_OUT_LEN);
    QString srchash = "";
    for(size_t i=0; i < BLAKE3_OUT_LEN; i++)
    {
        srchash.append(QString("%1").arg(output[i], 2, 16, QChar('0')));
    }
    srchash += "," + filename;

    return srchash;
}

void WriteHash(QString hashstring)
{
    QString hash = hashstring.split(",").at(0);
    QString file = hashstring.split(",").at(1);
    /*
    MDB_dbi ldbi;
    MDB_val key, data;
    MDB_txn* txn;
    rc = mdb_txn_begin(lenv, NULL, 0, &txn);
    rc = mdb_dbi_open(txn, NULL, 0, &ldbi);
    key.mv_size = hash.size();
    key.mv_data = hash.data();
    data.mv_size = file.size();
    data.mv_data = file.data();
    rc = mdb_put(txn, ldbi, &key, &data, 0);
    mdb_txn_commit(txn);
    mdb_dbi_close(lenv, ldbi);
    */
    hashfile.open(QIODevice::WriteOnly | QIODevice::Append | QIODevice::Text);
    hashfile.write(hashstring.toStdString().c_str());
    hashfile.write("\n");
    hashfile.close();
}

int main(int argc, char* argv[])
{
    QCoreApplication app(argc, argv);
    QCoreApplication::setApplicationName("wombathasher");
    QCoreApplication::setApplicationVersion("0.1");
    QCommandLineParser parser;
    parser.setApplicationDescription("Generate a Hash List or compare files against a hash list.");
    parser.addHelpOption();
    parser.addVersionOption();
    parser.addPositionalArgument("files", QCoreApplication::translate("main", "Files to hash for comparison or to add to a hash list."));
    QCommandLineOption createlistoption(QStringList() << "c", QCoreApplication::translate("main", "Create New Hash List"), QCoreApplication::translate("main", "new list"));
    QCommandLineOption appendlistoption(QStringList() << "a", QCoreApplication::translate("main", "Append to Existing Hash List"), QCoreApplication::translate("main", "existing list"));
    QCommandLineOption recurseoption(QStringList() << "r", QCoreApplication::translate("main", "Recurse sub-directories"));
    QCommandLineOption knownoption(QStringList() << "k", QCoreApplication::translate("main", "Compare computed hashes to known list"), QCoreApplication::translate("main", "file"));
    QCommandLineOption matchoption(QStringList() << "m", QCoreApplication::translate("main", "Matching mode. Requires -k"));
    QCommandLineOption negmatchoption(QStringList() << "n", QCoreApplication::translate("main", "Negative (Inverse) matching mode. Requires -k"));
    QCommandLineOption matchedfileoption(QStringList() << "w", QCoreApplication::translate("main", "Display which known file was matched, requires -m"));
    QCommandLineOption relpathoption(QStringList() << "l", QCoreApplication::translate("main", "Print relative paths for filenames."));

    parser.addOption(createlistoption);
    parser.addOption(appendlistoption);
    parser.addOption(recurseoption);
    parser.addOption(knownoption);
    parser.addOption(matchoption);
    parser.addOption(negmatchoption);
    parser.addOption(matchedfileoption);
    parser.addOption(relpathoption);

    parser.process(app);

    QString createlistname = parser.value(createlistoption);
    QString appendlistname = parser.value(appendlistoption);
    bool recursebool = parser.isSet(recurseoption);
    QString knownlistfile = parser.value(knownoption);
    bool matchbool = parser.isSet(matchoption);
    bool negmatchbool = parser.isSet(negmatchoption);
    bool matchedfilebool = parser.isSet(matchedfileoption);
    bool relpathbool = parser.isSet(relpathoption);
    /*
    qDebug() << "Create hash list name:" << createlistname;
    qDebug() << "Append to hash list:" << appendlistname;
    qDebug() << "Is Recursive mode set:" << recursebool;
    qDebug() << "Known list file path:" << knownlistfile;
    qDebug() << "Is Matching mode set:" << matchbool;
    qDebug() << "Is Negative matching mode set:" << negmatchbool;
    qDebug() << "relative path set:" << relpathbool;
    */


    const QStringList args = parser.positionalArguments();
    if(args.count() <= 0)
    {
        qInfo() << "No files provided for hashing.";
        return 1;
    }
    //qDebug() << "files to hash:" << args;

    // IF FILE ARG IS A DIRECTORY AND -R ISN'T SET, THEN OMIT HASHING THE DIRECTORY AND DON'T GET IT'S CONTENTS...
    // IF -c,-a,-k is not set, then just hash the files and spit out the blake3 hash, absolute file path

    QStringList filelist;
    filelist.clear();
    QStringList hashlist;
    hashlist.clear();
    QString hashlistname = "";

    for(int i=0; i < args.count(); i++)
    {
	QFileInfo curfi(args.at(i));
	if(curfi.isDir())
	{
	    if(recursebool)
	    {
		DirectoryRecurse(args.at(i), &filelist, relpathbool);
	    }
	    else
		qInfo() << "Skipped the directory:" << args.at(i) << ", -r to hash the contents of the directory.";
	}
	else if(curfi.isFile())
	{
            if(relpathbool)
                filelist.append(curfi.filePath());
            else
                filelist.append(curfi.absoluteFilePath());
	    //filelist.append(args.at(i));
	}
    }
    if(filelist.count() > 0)
	hashlist = QtConcurrent::blockingMapped(filelist, HashFiles);

    // GOT FILE LIST TO OPERATE ON AND NOW I NEED TO PROCESS
    if(parser.isSet(createlistoption) && parser.isSet(appendlistoption))
    {
        qInfo() << "Cannot use -c and -a.";
        return 1;
    }
    else if(matchbool)
    {
        // check for -k -w -n here.
        qDebug() << "-m provided";
    }
    else if(negmatchbool)
    {
        // check for -k -w -n here.
        qDebug() << "-n provided";
    }
    else
    {
        /*
        QString knownlistfile = parser.value(knownoption);
        bool matchbool = parser.isSet(matchoption);
        bool negmatchbool = parser.isSet(negmatchoption);
        bool matchedfilebool = parser.isSet(matchedfileoption);
         */ 
        if(parser.isSet(appendlistoption))
        {
            if(appendlistname.isEmpty() || appendlistname.isNull())
            {
                qInfo() << "No hash list name provided.";
                return 1;
            }
            else
                hashlistname = appendlistname;
        }
        if(parser.isSet(createlistoption))
        {
            if(createlistname.isEmpty() || createlistname.isNull())
            {
                qInfo() << "No hash list name provided.";
                return 1;
            }
            else
                hashlistname = createlistname;
        }
        if(!hashlistname.isEmpty() && !hashlistname.isNull())
        {

            // CREATE HASH FILE TXT FILE
            hashfile.setFileName(hashlistname + ".whl");
            hashfile.open(QIODevice::WriteOnly | QIODevice::Text | QIODevice::Append);
            hashfile.close();

            /*
            rc = mdb_env_create(&lenv);
            QDir tmpdir;
            tmpdir.mkpath(hashlistname);
            rc = mdb_env_open(lenv, hashlistname.toStdString().c_str(), 0, 0664);
            */

            //if(parser.isSet(createlistoption) && rc == 0)
            if(parser.isSet(createlistoption))
                qInfo() << "Hash List Successfully created." << Qt::endl;
            //if(parser.isSet(appendlistoption) && rc == 0)
            if(parser.isSet(appendlistoption))
                qInfo() << "Hash List Successfully opened." << Qt::endl;
            for(int i=0; i < hashlist.count(); i++)
            {
                WriteHash(hashlist.at(i));
                //qDebug() << hashlist.at(i);
            }
            qInfo() << hashlist.count() << "hashes written to the hash list file.";
        }
    }

    return 0;
}
