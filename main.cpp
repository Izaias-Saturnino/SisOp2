#include <stdio.h>
#include<unistd.h>
#include <stdlib.h>
#include <pthread.h>
#include <dirent.h>
#include <stdio.h>
#include <vector>
#include <string>
#include <fstream>
#include <filesystem>
#include <iostream>
#include <chrono>

using namespace std::chrono_literals;

#define TRUE 1

pthread_mutex_t m = PTHREAD_MUTEX_INITIALIZER;
const auto copyOptions = std::filesystem::copy_options::update_existing
                           | std::filesystem::copy_options::recursive;
void *folderchecker(void *arg)
{


   DIR *check;
   struct dirent *dir;
   std::vector<std::string> filesOld;
   std::vector<std::filesystem::file_time_type> timesOld;
   check = opendir("./in");
   while ((dir = readdir(check)) != NULL)
      {
         filesOld.push_back(dir->d_name);
         std::string path="./in/"+std::string(dir->d_name);
         timesOld.push_back(std::filesystem::last_write_time(path));

      }
      timesOld.pop_back();
      timesOld.pop_back();
   closedir(check);
   while (TRUE)
   {
      check = opendir("./in");
      std::vector<std::string> files;
      std::vector<std::filesystem::file_time_type> times;

      while ((dir = readdir(check)) != NULL)
      {
         files.push_back(dir->d_name);
         std::string path="./in/"+std::string(dir->d_name);
         times.push_back(std::filesystem::last_write_time(path));
      }
      times.pop_back();
      times.pop_back();
      if (files == filesOld && times==timesOld)
      {
         printf("no change\n");
      }else{
         printf("change\n");
         std::filesystem::remove_all("./output");
         std::filesystem::copy("./in","./output",copyOptions);   
         filesOld=files;
         timesOld=times;

      }
      closedir(check);
      sleep(1);
   }
}

int main(int argc, char *argv[])
{
   pthread_t thr1, thr2;
   int n1 = 1;

   pthread_create(&thr1, NULL, folderchecker, (void *)&n1);

   pthread_exit(0);
}