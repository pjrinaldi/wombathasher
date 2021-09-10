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
#include "lmdb.h"


void DirectoryRecurse(QString dirname, QStringList* filelist)
{
    QDir curdir(dirname);
    if(curdir.isEmpty())
	qDebug() << "dir" << dirname << "is empty and will be skipped.";
    else
    {
	QFileInfoList infolist = curdir.entryInfoList(QDir::NoDotAndDotDot | QDir::Files | QDir::Dirs);
	for(int i=0; i < infolist.count(); i++)
	{
	    QFileInfo tmpinfo = infolist.at(i);
	    //DirectoryRecurse();
	}
    }
    /*if(tmpfileinfo->isDir()) // its a directory, need to read its contents..
    {
        QDir tmpdir(tmpfileinfo->absoluteFilePath());
        if(tmpdir.isEmpty())
            qDebug() << "dir" << tmpfileinfo->fileName() << "is empty and will be skipped.";
        else
        {
            QFileInfoList infolist = tmpdir.entryInfoList(QDir::NoDotAndDotDot | QDir::Files | QDir::Dirs);
            for(int i=0; i < infolist.count(); i++)
            {
                QFileInfo tmpinfo = infolist.at(i);
                PopulateFile(&tmpinfo, blake3bool, catsigbool, out, logout);
            }
        }
    }

     */ 
}

QString HashFiles(QString filename)
{
    // calculate the blake3 hash here...
    //qDebug() << "file name:" << filename;

    return filename;
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
    //QCommandLineOption matchedfileoption(QStringList() << "w", QCoreApplication::translate("main", "Display which known file was matched, requires -m"));
    QCommandLineOption relpathoption(QStringList() << "l", QCoreApplication::translate("main", "Print relative paths for filenames."));

    parser.addOption(createlistoption);
    parser.addOption(appendlistoption);
    parser.addOption(recurseoption);
    parser.addOption(knownoption);
    parser.addOption(matchoption);
    parser.addOption(negmatchoption);
    //parser.addOption(matchedfileoption);
    parser.addOption(relpathoption);

    parser.process(app);

    QString createlistname = parser.value(createlistoption);
    QString appendlistname = parser.value(appendlistoption);
    bool recursebool = parser.isSet(recurseoption);
    QString knownlistfile = parser.value(knownoption);
    bool matchbool = parser.isSet(matchoption);
    bool negmatchbool = parser.isSet(negmatchoption);
    //bool matchedfilebool = parser.isSet(matchedfileoption);
    bool relpathbool = parser.isSet(relpathoption);
    qDebug() << "Create hash list name:" << createlistname;
    qDebug() << "Append to hash list:" << appendlistname;
    qDebug() << "Is Recursive mode set:" << recursebool;
    qDebug() << "Known list file path:" << knownlistfile;
    qDebug() << "Is Matching mode set:" << matchbool;
    qDebug() << "Is Negative matching mode set:" << negmatchbool;
    qDebug() << "relative path set:" << relpathbool;

    const QStringList args = parser.positionalArguments();
    qDebug() << "files to hash:" << args;

    // IF FILE ARG IS A DIRECTORY AND -R ISN'T SET, THEN OMIT HASHING THE DIRECTORY AND DON'T GET IT'S CONTENTS...
    // IF -c,-a,-k is not set, then just hash the files and spit out the blake3 hash, absolute file path

    QStringList filelist;
    filelist.clear();
    QStringList hashlist;
    hashlist.clear();
    //QFuture<QString> hashlist;

    for(int i=0; i < args.count(); i++)
    {
	QFileInfo curfi(args.at(i));
	if(curfi.isDir())
	{
	    if(recursebool)
	    {
		// NEED TO BUILD A RECURSIVE FUNCTION TO GO INTO EACH DIRECTORY AND ADD FILES TO THE FILE LIST...
		//DirectoryRecurse(args.at(i), &filelist);
		qDebug() << "go into direcetory and add files...";
	    }
	    else
		qInfo() << "Skipped the directory:" << args.at(i) << "-r to hash the contents of the directory.";
	}
	else if(curfi.isFile())
	{
	    filelist.append(args.at(i));
	    //qDebug() << curfi.filePath();
	}
    }
    if(filelist.count() > 0)
	hashlist = QtConcurrent::blockingMapped(filelist, HashFiles);

    for(int i=0; i < hashlist.count(); i++)
	qDebug() << "hash result item:" << hashlist.at(i);
    //for(int i=0; i < hashlist.resultCount(); i++)
	//qDebug() << "hash result item:" << hashlist.resultAt(i);

    return 0;
}

/*
void PopulateFile(QFileInfo* tmpfileinfo, bool blake3bool, bool catsigbool, QDataStream* out, QTextStream* logout)
{
    if(tmpfileinfo->isDir()) // its a directory, need to read its contents..
    {
        QDir tmpdir(tmpfileinfo->absoluteFilePath());
        if(tmpdir.isEmpty())
            qDebug() << "dir" << tmpfileinfo->fileName() << "is empty and will be skipped.";
        else
        {
            QFileInfoList infolist = tmpdir.entryInfoList(QDir::NoDotAndDotDot | QDir::Files | QDir::Dirs);
            for(int i=0; i < infolist.count(); i++)
            {
                QFileInfo tmpinfo = infolist.at(i);
                PopulateFile(&tmpinfo, blake3bool, catsigbool, out, logout);
            }
        }
    }
    else if(tmpfileinfo->isFile()) // its a file, so read the info i need...
    {
        // name << path << size << created << accessed << modified << status changed << b3 hash << category << signature
        // other info to gleam such as groupid, userid, permission, 
        *out << (qint64)0x776c69696e646578; // wombat logical image index entry
        *out << (QString)tmpfileinfo->fileName(); // FILENAME
        *out << (QString)tmpfileinfo->absolutePath(); // FULL PATH
        *out << (qint64)tmpfileinfo->size(); // FILE SIZE (8 bytes)
        *out << (qint64)tmpfileinfo->birthTime().toSecsSinceEpoch(); // CREATED (8 bytes)
        *out << (qint64)tmpfileinfo->lastRead().toSecsSinceEpoch(); // ACCESSED (8 bytes)
        *out << (qint64)tmpfileinfo->lastModified().toSecsSinceEpoch(); // MODIFIED (8 bytes)
        *out << (qint64)tmpfileinfo->metadataChangeTime().toSecsSinceEpoch(); // STATUS CHANGED (8 bytes)
        QFile tmpfile(tmpfileinfo->absoluteFilePath());
        if(!tmpfile.isOpen())
            tmpfile.open(QIODevice::ReadOnly);
        // Initialize Blake3 Hasher for block device and forensic image
        uint8_t sourcehash[BLAKE3_OUT_LEN];
        blake3_hasher blkhasher;
        blake3_hasher_init(&blkhasher);
        QString srchash = "";
        QString mimestr = "";
        while(!tmpfile.atEnd())
        {
            QByteArray tmparray = tmpfile.read(2048);
            if(blake3bool)
                blake3_hasher_update(&blkhasher, tmparray.data(), tmparray.count());
        }
        if(blake3bool)
        {
            blake3_hasher_finalize(&blkhasher, sourcehash, BLAKE3_OUT_LEN);
            for(size_t i=0; i < BLAKE3_OUT_LEN; i++)
            {
                srchash.append(QString("%1").arg(sourcehash[i], 2, 16, QChar('0')));
            }
        }
        *out << (QString)srchash; // BLAKE3 HASH FOR FILE CONTENTS OR EMPTY
        if(catsigbool)
        {
            if(tmpfileinfo->fileName().startsWith("$UPCASE_TABLE"))
                mimestr = "System File/Up-case Table";
            else if(tmpfileinfo->fileName().startsWith("$ALLOC_BITMAP"))
                mimestr = "System File/Allocation Bitmap";
            else if(tmpfileinfo->fileName().startsWith("$UpCase"))
                mimestr = "Windows System/System File";
            else if(tmpfileinfo->fileName().startsWith("$MFT") || tmpfileinfo->fileName().startsWith("$MFTMirr") || tmpfileinfo->fileName().startsWith("$LogFile") || tmpfileinfo->fileName().startsWith("$Volume") || tmpfileinfo->fileName().startsWith("$AttrDef") || tmpfileinfo->fileName().startsWith("$Bitmap") || tmpfileinfo->fileName().startsWith("$Boot") || tmpfileinfo->fileName().startsWith("$BadClus") || tmpfileinfo->fileName().startsWith("$Secure") || tmpfileinfo->fileName().startsWith("$Extend"))
                mimestr = "Windows System/System File";
            else
            {
                // NON-QT WAY USING LIBMAGIC
                tmpfile.seek(0);
                QByteArray sigbuf = tmpfile.read(2048);
                magic_t magical;
                const char* catsig;
                magical = magic_open(MAGIC_MIME_TYPE);
                magic_load(magical, NULL);
                catsig = magic_buffer(magical, sigbuf.data(), sigbuf.count());
                std::string catsigstr(catsig);
                mimestr = QString::fromStdString(catsigstr);
                magic_close(magical);
                for(int i=0; i < mimestr.count(); i++)
                {
                    if(i == 0 || mimestr.at(i-1) == ' ' || mimestr.at(i-1) == '-' || mimestr.at(i-1) == '/')
                        mimestr[i] = mimestr[i].toUpper();
                }
            }
            //else if(tmpfileinfo->fileName().startsWith("$INDEX_ROOT:") || tmpfileinfo->fileName().startsWith("$DATA:") || tmpfileinfo->fileName().startWith("$INDEX_ALLOCATION:"))
        }
        *out << (QString)mimestr; // CATEGORY/SIGNATURE STRING FOR FILE CONTENTS OR EMPTY
        *out << (qint8)5; // itemtype
        *out << (qint8)0; // deleted

        // ATTEMPT USING A SINGLE LZ4FRAME SIMILAR TO BLAKE3 HASHING
        tmpfile.seek(0);
        qint64 curpos = 0;
        size_t destsize = LZ4F_compressFrameBound(2048, NULL);
        char* dstbuf = new char[destsize];
        char* srcbuf = new char[2048];
        int dstbytes = 0;
        int compressedsize = 0;
        LZ4F_cctx* lz4cctx;
        LZ4F_errorCode_t errcode;
        errcode = LZ4F_createCompressionContext(&lz4cctx, LZ4F_getVersion());
        if(LZ4F_isError(errcode))
            qDebug() << "LZ4 Create Error:" << LZ4F_getErrorName(errcode);
        dstbytes = LZ4F_compressBegin(lz4cctx, dstbuf, destsize, NULL);
        compressedsize += dstbytes;
        if(LZ4F_isError(dstbytes))
            qDebug() << "LZ4 Begin Error:" << LZ4F_getErrorName(dstbytes);
        out->writeRawData(dstbuf, dstbytes);
        while(curpos < tmpfile.size())
        {
            int bytesread = tmpfile.read(srcbuf, 2048);
            dstbytes = LZ4F_compressUpdate(lz4cctx, dstbuf, destsize, srcbuf, bytesread, NULL);
            if(LZ4F_isError(dstbytes))
                qDebug() << "LZ4 Update Error:" << LZ4F_getErrorName(dstbytes);
            dstbytes = LZ4F_flush(lz4cctx, dstbuf, destsize, NULL);
            if(LZ4F_isError(dstbytes))
                qDebug() << "LZ4 Flush Error:" << LZ4F_getErrorName(dstbytes);
            compressedsize += dstbytes;
            out->writeRawData(dstbuf, dstbytes);
            curpos = curpos + bytesread;
            //printf("Wrote %llu of %llu bytes\r", curpos, totalbytes);
            //fflush(stdout);
        }
        dstbytes = LZ4F_compressEnd(lz4cctx, dstbuf, destsize, NULL);
        compressedsize += dstbytes;
        out->writeRawData(dstbuf, dstbytes);
        delete[] srcbuf;
        delete[] dstbuf;
        errcode = LZ4F_freeCompressionContext(lz4cctx);
        tmpfile.close();
        *logout << "Processed:" << tmpfileinfo->absoluteFilePath() << Qt::endl;
        qDebug() << "Processed: " << tmpfileinfo->fileName();
    }
}
*/
