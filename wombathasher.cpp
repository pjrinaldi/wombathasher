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
#include <fstream>
#include <thread>

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

void HashFile(std::string filename, std::string whlfile)
{
    std::ifstream fin(filename.c_str());
    char tmpchar[65536];
    blake3_hasher hasher;
    blake3_hasher_init(&hasher);
    while(fin)
    {
	fin.read(tmpchar, 65536);
	size_t cnt = fin.gcount();
	blake3_hasher_update(&hasher, tmpchar, cnt);
	if(!cnt)
	    break;
    }
    uint8_t output[BLAKE3_OUT_LEN];
    blake3_hasher_finalize(&hasher, output, BLAKE3_OUT_LEN);
    std::stringstream ss;
    for(int i=0; i < BLAKE3_OUT_LEN; i++)
        ss << std::hex << (int)output[i]; 
    std::string srcmd5 = ss.str();
    std::string whlstr = srcmd5 + "," + filename + "\n";
    FILE* whlptr = NULL;
    whlptr = fopen(whlfile.c_str(), "a");
    fwrite(whlstr.c_str(), strlen(whlstr.c_str()), 1, whlptr);
    fclose(whlptr);
}

void CompareFile(std::string filename, std::map<std::string, std::string>* knownhashes, int8_t matchbool)
{
    std::ifstream fin(filename.c_str());
    char tmpchar[65536];
    blake3_hasher hasher;
    blake3_hasher_init(&hasher);
    while(fin)
    {
	fin.read(tmpchar, 65536);
	size_t cnt = fin.gcount();
	blake3_hasher_update(&hasher, tmpchar, cnt);
	if(!cnt)
	    break;
    }
    uint8_t output[BLAKE3_OUT_LEN];
    blake3_hasher_finalize(&hasher, output, BLAKE3_OUT_LEN);
    std::stringstream ss;
    for(int i=0; i < BLAKE3_OUT_LEN; i++)
        ss << std::hex << (int)output[i]; 
    std::string srcmd5 = ss.str();
    
    std::string matchstring = filename;
    uint8_t hashash = knownhashes->count(srcmd5);
    if(matchbool == 0 && hashash == 1)
    {
	matchstring += " matches " + knownhashes->at(srcmd5) + ".\n";
	std::cout << matchstring;
    }
    if(matchbool == 1 && hashash == 0)
    {
	matchstring += " does not match known files.\n";
	std::cout << matchstring;
    }
}

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
    std::map<std::string, std::string> knownhashes;
    knownhashes.clear();

    //unsigned int threadcount = std::thread::hardware_concurrency();
    //std::cout << threadcount << " concurrent threads are supported.\n";

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
    if(isnew || isappend)
    {
        std::string whlstr;
        if(isnew) // create a new whl file
        {
            whlstr = newpath.string();
            std::size_t found = whlstr.rfind(".whl");
            if(found == -1)
                whlstr += ".whl";
        }
        if(isappend) // append to existing whl file
            whlstr = appendpath.string();
	for(int i=0; i < filelist.size(); i++)
	{
	    std::thread tmp(HashFile, filelist.at(i).string(), whlstr);
	    tmp.join();
	}
    }
    if(isknown)
    {
        // READ KNOWN HASH LIST FILE INTO A MAP
	std::string matchstring = "";
        std::ifstream knownstream;
        knownstream.open(knownpath.string(), std::ios::in);
        std::string tmpfile;
        while(std::getline(knownstream, tmpfile))
	{
	    std::size_t found = tmpfile.find(",");
	    std::string tmpkey = tmpfile.substr(0, found);
	    std::string tmpval = tmpfile.substr(found+1);
	    knownhashes.insert(std::pair<std::string, std::string>(tmpkey, tmpval));
	    //std::cout << tmpkey << " | " << tmpval << "\n";
	}
        knownstream.close();
	int8_t matchbool = -1;
	if(ismatch)
	    matchbool = 0;
	if(isnotmatch)
	    matchbool = 1;

	for(int i=0; i < filelist.size(); i++)
	{
	    std::thread tmp(CompareFile, filelist.at(i).string(), &knownhashes, matchbool);
	    tmp.join();
	}
    }

    return 0;
}
