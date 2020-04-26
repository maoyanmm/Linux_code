#include<stdio.h>
#include<unistd.h>
int main()
{
   // execl("/usr/bin/ls","ls","-l",NULL);
   //for(int i = 0;i < argc; ++i)
   //{
   //    printf("[%d]:[%s]\n",i,argv[i]);
   //}
   //for(int i = 0;env[i]; ++i){
   //    printf("%s\n",env[i]);
   //}
   char* argv[3];
   argv[0] = "ls";
   argv[1] = "-l";
   argv[2] = NULL;
   execv("/usr/bin/ls",argv);
    return 0;
}
