//move to dir manager
bool create_folder(string folder_path){
    bool folder_created = mkdir(folder_path.c_str(), 0777) == -1;
    return folder_created;
}

//move to dir manager
//lista de arquivos só da primeira pasta (não é recursiva)
vector<string> get_file_list(string path){
    vector<string> list;
    for (const auto & entry : fs::directory_iterator(path))
        list.push_back(entry.path());
    return list;
}

//move to dir manager
vector<string> get_removed_files_list(vector<string> server_file_list, vector<string> client_file_list){
    //verifica os arquivos que estão no diretório do cliente, mas não no servidor e coloca o nome deles na lista
}

vector<string> get_new_files_list(vector<string> server_file_list, vector<string> client_file_list){
    //verifica os arquivos que estão no servidor, mas faltam para o cliente e coloca o nome deles na lista
}

//move to dir manager
//remove o aquivo do servidor
int delete_file(string file_path){
    const char *c = file_path.c_str();
    int result = remove(c);

    return result;
}

//move to dir manager
void create_file_from_request(){
    //cria um arquivo a partir do payload request
}