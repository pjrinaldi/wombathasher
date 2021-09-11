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

QFile hashfile;
int rc;
MDB_env* lenv;

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
    //bool matchbool = parser.isSet(matchoption);
    //bool negmatchbool = parser.isSet(negmatchoption);
    //bool matchedfilebool = parser.isSet(matchedfileoption);
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
	    filelist.append(args.at(i));
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
    else
    {
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
            hashfile.open(QIODevice::WriteOnly);
            hashfile.close();

            rc = mdb_env_create(&lenv);
            QDir tmpdir;
            tmpdir.mkpath(hashlistname);
            rc = mdb_env_open(lenv, hashlistname.toStdString().c_str(), 0, 0664);

            if(parser.isSet(createlistoption) && rc == 0)
                qInfo() << "Hash List Successfully created." << Qt::endl;
            if(parser.isSet(appendlistoption) && rc == 0)
                qInfo() << "Hash List Successfully opened." << Qt::endl;
        }
    }

    for(int i=0; i < hashlist.count(); i++)
    {
        WriteHash(hashlist.at(i));
        //qDebug() << hashlist.at(i);
    }

    // OUTPUT CHECK...
    MDB_cursor* cursor;
    MDB_dbi ldbi;
    MDB_val key, data;
    MDB_txn* txn;
    rc = mdb_dbi_open(txn, NULL, 0, &ldbi);
    if(rc != 0)
    {
        qInfo() << "dbi open error";
        return 1;
    }
    rc = mdb_txn_begin(lenv, NULL, MDB_RDONLY, &txn);
    if(rc != 0)
    {
        qDebug() << "txn begin error";
        return 1;
    }
    rc = mdb_cursor_open(txn, ldbi, &cursor);
    if(rc != 0)
    {
        qInfo() << "cursor open error";
        return 1;
    }
    while ((rc = mdb_cursor_get(cursor, &key, &data, MDB_NEXT)) == 0)
    {
        printf("key: %p %.*s, data: %p %.*s\n", key.mv_data,  (int) key.mv_size,  (char *) key.mv_data, data.mv_data, (int) data.mv_size, (char *) data.mv_data);
    }
    mdb_cursor_close(cursor);
    mdb_txn_abort(txn);

    return 0;
}

/*
	// Note: Most error checking omitted for simplicity
	rc = mdb_txn_begin(env, NULL, 0, &txn);
	rc = mdb_dbi_open(txn, NULL, 0, &dbi);

	rc = mdb_put(txn, dbi, &key, &data, 0);
	rc = mdb_txn_commit(txn);
	if (rc) {
		fprintf(stderr, "mdb_txn_commit: (%d) %s\n", rc, mdb_strerror(rc));
		goto leave;
	}
	rc = mdb_txn_begin(env, NULL, MDB_RDONLY, &txn);
	rc = mdb_cursor_open(txn, dbi, &cursor);
	while ((rc = mdb_cursor_get(cursor, &key, &data, MDB_NEXT)) == 0) {
		printf("key: %p %.*s, data: %p %.*s\n",
			key.mv_data,  (int) key.mv_size,  (char *) key.mv_data,
			data.mv_data, (int) data.mv_size, (char *) data.mv_data);
	}
	mdb_cursor_close(cursor);
	mdb_txn_abort(txn);
leave:
	mdb_dbi_close(env, dbi);
	mdb_env_close(env);
	return 0;
*/

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
