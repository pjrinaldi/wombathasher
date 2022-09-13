#include <stdint.h>
#include <stdio.h>
#include <unistd.h>
#include <errno.h>
#include <stdio.h>
#include <time.h>
#include <string.h>
#include <stdlib.h>

#include <map>
//#include <iostream>
#include "blake3.h"

FILE* hashfile = NULL;


/*
QFile hashfile;
QHash<QString, QString> knownhashes;
quint8 matchtype = 0;
bool matchedfilebool = false;

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
    hashfile.open(QIODevice::WriteOnly | QIODevice::Append | QIODevice::Text);
    QTextStream out;
    out.setDevice(&hashfile);
    out << hashstring << Qt::endl;
    hashfile.close();
}

QString HashCompare(QString unknownhashentry)
{
    QString unkhash = unknownhashentry.split(",").at(0);
    QString unkfile = unknownhashentry.split(",").at(1);
    QString matchstring = "";
    if(matchtype == 0)
    {
        if(knownhashes.contains(unkhash))
        {
            matchstring = unkfile;
            if(matchedfilebool)
                matchstring += " matches " + knownhashes.value(unkhash);
        }
    }
    else if(matchtype == 1)
    {
        if(!knownhashes.contains(unkhash))
            matchstring = unkfile;
    }

    return matchstring;
}
*/

void ShowUsage(int outtype)
{
    if(outtype == 0)
    {
        printf("Generates a Wombat Hash List (whl) or compare files against a hash list.\n\n");
        printf("Usage :\n");
        printf("\twombathasher [OPTIONS] files\n\n");
        printf("Options :\n");
	printf("-c <new list>\t: Create new hash list.\n");
	printf("-a <existing list>\t: Append to an existing hash list.\n");
	printf("-r\t: Recurse sub-directories.\n");
	printf("-k <file>\t: Compare computed hashes to known list.\n");
	printf("-m\t: Matching mode. Requires -k.\n");
	printf("-n\t: Negative (Inverse) matching mode. Requires -k.\n");
	printf("-w\t: Display which known file was matched, requires -m.\n");
	printf("-l\t: Print relative paths for filenames.\n");
        printf("-v\t: Prints Version information\n");
        printf("-h\t: Prints help information\n\n");
	printf("Arguments:\n");
	printf("files\t: Files to hash for comparison or to add to a hash list.\n");
        //printf("Example Usage :\n");
        //printf("wombatimager /dev/sda item1 -v -c \"Case 1\" -e \"My Name\" -n \"Item 1\" -d \"Forensic Image of a 32MB SD Card\"\n");
    }
    else if(outtype == 1)
    {
        printf("wombatimager v0.1\n");
	printf("License CC0-1.0: Creative Commons Zero v1.0 Universal\n");
        printf("This software is in the public domain\n");
        printf("There is NO WARRANTY, to the extent permitted by law.\n\n");
        printf("Written by Pasquale Rinaldi\n");
    }
};

int main(int argc, char* argv[])
{
    if(argc == 1 || (argc == 2 && strcmp(argv[1], "-h") == 0))
    {
	ShowUsage(0);
	return 1;
    }
    else if(argc == 2 && strcmp(argv[1], "-v") == 0)
    {
	ShowUsage(1);
	return 1;
    }
    else if(argc >= 3)
    {
        for(int i=0; i < argc; i++)
        {
	    if(strchr(argv[i], '-') == NULL)
		printf("argument: %d option\n", i);
            printf("Command option %d, %s\n", i, argv[i]);
	    /*
            if(strcmp(argv[i], "-v") == 0)
            {
                verify=1;
                printf("verification is set\n");
            }
            else if(strcmp(argv[i], "-c") == 0)
                strcpy(wfimd.casenumber, argv[i+1]);
            else if(strcmp(argv[i], "-e") == 0)
                strcpy(wfimd.examiner, argv[i+1]);
            else if(strcmp(argv[i], "-n") == 0)
                strcpy(wfimd.evidencenumber, argv[i+1]);
            else if(strcmp(argv[i], "-d") == 0)
                strcpy(wfimd.description, argv[i+1]);
            else if(strcmp(argv[i], "-V") == 0)
            {
                ShowUsage(1);
                return 1;
            }
            else if(strcmp(argv[i], "-h") == 0)
            {
                ShowUsage(0);
                return 1;
            }
	    */
        }
	//printf("Command called: %s %s %s\n", argv[0], argv[1], argv[2]);
    }

    /*
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
    matchedfilebool = parser.isSet(matchedfileoption);
    bool relpathbool = parser.isSet(relpathoption);

    const QStringList args = parser.positionalArguments();
    if(args.count() <= 0)
    {
        qInfo() << "No files provided for hashing.\n";
        parser.showHelp(1);
        //return 1;
    }

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
    else if(matchbool || negmatchbool)
    {
        if(matchbool && negmatchbool)
        {
            qInfo() << "Cannot use -m and -n at the same time.";
            return 1;
        }
        if(matchbool)
            matchtype = 0;
        if(negmatchbool)
            matchtype = 1;
        if(!parser.isSet(knownoption))
        {
            qInfo() << "-k required when using -m";
            return 1;
        }
        knownhashes.clear();
        QFile comparefile(parser.value(knownoption));
        if(!comparefile.isOpen())
            comparefile.open(QIODevice::ReadOnly | QIODevice::Text);
        while(!comparefile.atEnd())
        {
            QString hashentry = QString(comparefile.readLine());
            knownhashes.insert(hashentry.split(",").at(0), hashentry.split(",").at(1));
        }
        comparefile.close();
        QStringList hashcomparelist = QtConcurrent::blockingMapped(hashlist, HashCompare);
        if(hashcomparelist.count() > 0)
        {
            for(int i=0; i < hashcomparelist.count(); i++)
            {
                if(!hashcomparelist.at(i).isEmpty())
                    qDebug() << hashcomparelist.at(i);
            }
        }
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
            if(!hashlistname.endsWith(".whl"))
                hashlistname += ".whl";
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
            if(!hashlistname.endsWith(".whl"))
                hashlistname += ".whl";
        }
        if(!hashlistname.isEmpty() && !hashlistname.isNull())
        {
            // CREATE HASH FILE TXT FILE
            hashfile.setFileName(hashlistname);
            hashfile.open(QIODevice::WriteOnly | QIODevice::Text | QIODevice::Append);
            hashfile.close();
            for(int i=0; i < hashlist.count(); i++)
            {
                WriteHash(hashlist.at(i));
            }
            qInfo() << hashlist.count() << "hashes written to the" << hashlistname << "hash list file.";
        }
    }
    if(!parser.isSet(appendlistoption) && !parser.isSet(createlistoption) && !matchbool && !negmatchbool && !matchedfilebool && !parser.isSet(knownoption))
    {
        for(int i=0; i < hashlist.count(); i++)
        {
            qInfo() << hashlist.at(i);
        }
    }
    */

    return 0;
}
