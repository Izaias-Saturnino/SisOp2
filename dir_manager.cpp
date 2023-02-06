#include <string>
#include <vector>
#include <sys/stat.h>
#include <unistd.h>
#include <filesystem>
#include <iostream>
#include <algorithm>

namespace fs = std::filesystem;
using namespace std;

//cria folder

bool create_folder(string folder_path){
    bool folder_created = mkdir(folder_path.c_str(), 0777) == -1;
    return folder_created;
}

//lista de arquivos só da primeira pasta (não é recursiva)
vector<string> get_file_list(string path){
    vector<string> list;
    for (const auto & entry : fs::directory_iterator(path))
        list.push_back(entry.path().string());
    return list;
}
//imprime arquivos e seu mac
vector<string> print_file_list(string path){
    struct stat fileInfo;
    vector <string> result;
    for (const auto & entry : fs::directory_iterator(path)){
        lstat(entry.path().c_str(), &fileInfo);
        string fileInfoString="File:"+ entry.path().string()+"\nCreated: "+ctime(&fileInfo.st_ctime)+"Modified: "+ ctime(&fileInfo.st_mtime)+"Last Access: "+ctime(&fileInfo.st_atime);
        result.push_back(fileInfoString);
        cout << "File: " << entry.path() << "\n";
        cout << "Created: " << ctime(&fileInfo.st_ctime);
        cout << "Modified: " << ctime(&fileInfo.st_mtime);
        cout << "Last Access: " << ctime(&fileInfo.st_atime);
    }
    return result;
}
vector<string> remove_path(vector<string> folderA){
    vector<string> list;
    for(int i=0;i<folderA.size();i++){
        list.push_back(folderA.at(i).substr(folderA.at(i).find_last_of("/")+1));
    }
    return list;
}

//ve que arquivos do folder a nao estao no folder b
vector<string> get_folder_diference(vector<string> folderA, vector<string> folderB){
    vector<string> list;
    vector<string> listA = remove_path(folderA);
    vector<string> listB = remove_path(folderB);
    for(int i=0;i<folderA.size();i++){
    if (!count(listB.begin(), listB.end(), listA.at(i))) {
        list.push_back(listA.at(i));
    }
    }
    return list;
}

//remove o aquivo do servidor
int delete_file(string file_path){
    const char *c = file_path.c_str();
    int result = remove(c);

    return result;
}

void create_file_from_request(){
    //cria um arquivo a partir do payload request
}

