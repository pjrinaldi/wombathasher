#include <stdint.h>
#include <stdio.h>
#include <unistd.h>
#include <errno.h>
#include <stdio.h>
#include <time.h>
#include <string.h>
#include <stdlib.h>

#include <map>
#include <vector>
#include <string>
#include <filesystem>
#include <iostream>

#include "blake3.h"

void ParseDirectory(std::filesystem::path dirpath, std::vector<std::filesystem::path>* filelist, uint8_t isrelative)
{
    for(auto const& dir_entry : std::filesystem::recursive_directory_iterator(dirpath))
    {
	if(std::filesystem::is_regular_file(dir_entry))
        {
            if(isrelative)
                filelist->push_back(std::filesystem::relative(dir_entry, std::filesystem::current_path()));
            else
                filelist->push_back(dir_entry);
        }
    }
}

/*
QFile hashfile;
QHash<QString, QString> knownhashes;
quint8 matchtype = 0;
bool matchedfilebool = false;

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
    //FILE* hashfile = NULL;
    uint8_t isnew = 0;
    uint8_t isappend = 0;
    uint8_t isrecursive = 0;
    uint8_t isknown = 0;
    uint8_t ismatch = 0;
    uint8_t isnotmatch = 0;
    uint8_t isdisplay = 0;
    uint8_t isrelative = 0;

    std::string newwhlstr = "";
    std::string appendwhlstr = "";
    std::string knownwhlstr = "";
    std::filesystem::path newpath;
    std::filesystem::path appendpath;
    std::filesystem::path knownpath;

    std::vector<std::filesystem::path> filevector;
    filevector.clear();
    std::vector<std::filesystem::path> filelist;
    filelist.clear();

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
        for(int i=1; i < argc; i++)
        {
	    /*
	    if(strchr(argv[i], '-') == NULL)
		printf("argument: %d option\n", i);
            printf("Command option %d, %s\n", i, argv[i]);
	    */

            if(strcmp(argv[i], "-v") == 0)
            {
                ShowUsage(1);
                return 1;
            }
            else if(strcmp(argv[i], "-h") == 0)
            {
                ShowUsage(0);
                return 1;
            }
            else if(strcmp(argv[i], "-c") == 0)
            {
                isnew = 1;
                newwhlstr = argv[i+1];
                i++;
            }
            else if(strcmp(argv[i], "-a") == 0)
            {
                isappend = 1;
                appendwhlstr = argv[i+1];
                i++;
            }
            else if(strcmp(argv[i], "-r") == 0)
                isrecursive = 1;
            else if(strcmp(argv[i], "-k") == 0)
            {
                isknown = 1;
                knownwhlstr = argv[i+1];
                i++;
            }
            else if(strcmp(argv[i], "-m") == 0)
                ismatch = 1;
            else if(strcmp(argv[i], "-n") == 0)
                isnotmatch = 1;
            else if(strcmp(argv[i], "-w") == 0)
                isdisplay = 1;
            else if(strcmp(argv[i], "-l") == 0)
                isrelative = 1;
            else
            {
		filevector.push_back(std::filesystem::canonical(argv[i]));
                //printf("part of files to hash.... %d %s\n", i, argv[i]);
            }
        }
	// ALSO NEED TO CHECK THE ARGUMENTS IF THEY ARE COMBINED AS IN -rmwl

	if(isnew)
        {
            /*
            std::size_t found = filestr.find_last_of("/");
            std::string pathname = filestr.substr(0, found);
            std::string filename = filestr.substr(found+1);
            std::filesystem::path initpath = std::filesystem::canonical(pathname + "/");
            imagepath = initpath.string() + "/" + filename + ".wfi";
            */
	    newpath = std::filesystem::absolute(newwhlstr);
        }
	if(isappend)
        {
	    appendpath = std::filesystem::absolute(appendwhlstr);
        }
	if(isknown)
        {
	    knownpath = std::filesystem::absolute(knownwhlstr);
        }
        // CHECK FOR INCOMPATIBLE OPTIONS
	if(isnew && isappend)
	{
	    printf("You cannot create a new and append to an existing wombat hash list (whl) file.\n");
	    return 1;
	}
	if(ismatch && !isknown || ismatch && isnotmatch || isnotmatch && !isknown)
	{
	    printf("(Not) Match requires a known whl file, cannot match/not match at the same time\n");
	    return 1;
	}
	if(isnew && std::filesystem::exists(newwhlstr))
	{
	    printf("Wombat Hash List (whl) file %s already exists. Cannot Create, try to append (-a)\n", newwhlstr.c_str());
	    return 1;
	}
	if(isappend && !std::filesystem::exists(appendwhlstr))
	{
	    printf("Wombat Hash List (whl) file %s does not exist. Cannot Append, try to create (-c)\n", appendwhlstr.c_str());
	    return 1;
	}
	if(isknown && !std::filesystem::exists(knownwhlstr))
	{
	    printf("Known wombat hash list (whl) file %s does not exist\n", knownwhlstr.c_str());
	    return 1;
	}

	for(int i=0; i < filevector.size(); i++)
	{
	    if(std::filesystem::is_regular_file(filevector.at(i)))
	    {
                if(isrelative)
                    filelist.push_back(std::filesystem::relative(filevector.at(i), std::filesystem::current_path()));
                else
                    filelist.push_back(filevector.at(i));
	    }
	    else if(std::filesystem::is_directory(filevector.at(i)))
	    {
		if(isrecursive)
		    ParseDirectory(filevector.at(i), &filelist, isrelative);
		else
		    printf("Directory %s skipped. Use -r to recurse directory\n", filevector.at(i).c_str());
	    }
	}
    }
    // GOT THE LIST OF FIlES (filelist), NOW I NEED TO HASH AND HANDLE ACCORDING TO OPTIONS 
    if(isnew) // create/open whl file
    {
        std::string whlfile = newpath.string();
        std::size_t found = whlfile.rfind(".whl");
        if(found == -1)
            whlfile += ".whl";
        /*
            // CREATE HASH FILE TXT FILE
            hashfile.setFileName(hashlistname);
            hashfile.open(QIODevice::WriteOnly | QIODevice::Text | QIODevice::Append);
            hashfile.close();
            for(int i=0; i < hashlist.count(); i++)
            {
                WriteHash(hashlist.at(i));
            }
            qInfo() << hashlist.count() << "hashes written to the" << hashlistname << "hash list file.";
        */
    }

    return 0;
}

    /*
// CPP program to demonstrate multithreading
// using three different callables.
#include <iostream>
#include <thread>
using namespace std;
  
// A dummy function
void foo(int Z)
{
    for (int i = 0; i < Z; i++) {
        cout << "Thread using function"
               " pointer as callable\n";
    }
}
  
// A callable object
class thread_obj {
public:
    void operator()(int x)
    {
        for (int i = 0; i < x; i++)
            cout << "Thread using function"
                  " object as  callable\n";
    }
};
  
int main()
{
    cout << "Threads 1 and 2 and 3 "
         "operating independently" << endl;
  
    // This thread is launched by using 
    // function pointer as callable
    thread th1(foo, 3);
  
    // This thread is launched by using
    // function object as callable
    thread th2(thread_obj(), 3);
  
    // Define a Lambda Expression
    auto f = [](int x) {
        for (int i = 0; i < x; i++)
            cout << "Thread using lambda"
             " expression as callable\n";
    };
  
    // This thread is launched by using 
    // lamda expression as callable
    thread th3(f, 3);
  
    // Wait for the threads to finish
    // Wait for thread t1 to finish
    th1.join();
  
    // Wait for thread t2 to finish
    th2.join();
  
    // Wait for thread t3 to finish
    th3.join();
  
    return 0;
}
     */

    /*

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
