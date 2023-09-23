#include <pwd.h>
#include <iostream>
#include <string.h>
#include <crypt.h>
#include <shadow.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>
#include <pwd.h>
#include <shadow.h>
#include <sys/time.h>
#include <errno.h>
#include <chrono>
#include <filesystem>
#include<grp.h>
#include <stdio.h>
#include <dirent.h>

//g++ -o user user.c -lcrypt
using namespace std;
/** user_t data type */
#define USER_ERROR_SIZE 256
#define USER_GROUP_ID   1001
#define USER_DEFAULT_ID   1001

/// Seconds in a day
#define DAY (24L*3600L)
/// Seconds in a week
#define WEEK (7*DAY)
/// Time scale used
#define SCALE DAY
/***********************************************************************************/
namespace fs = std::filesystem;

//function copy files
void cpFile(const fs::path & srcPath,
  const fs::path & dstPath, const char *__file, __uid_t __owner, __gid_t __group) {

  std::ifstream srcFile(srcPath, std::ios::binary);
  std::ofstream dstFile(dstPath, std::ios::binary);

  if (!srcFile || !dstFile) {
    std::cout << "Failed to get the file." << std::endl;
    return;
  }

  dstFile << srcFile.rdbuf();

  srcFile.close();
  dstFile.close();

  chown(dstPath.c_str(), __owner, __owner);
  chmod(dstPath.c_str(), 0755);

}

//function create new directory
void cpDirectory(const fs::path & srcPath,
  const fs::path & dstPath,const char *__file, __uid_t __owner, __gid_t __group) {

  fs::create_directories(dstPath);
  chown(dstPath.c_str(), __owner, __owner);
    chmod(dstPath.c_str(), 0755);
  for (const auto & entry: fs::directory_iterator(srcPath)) {
    const fs::path & srcFilePath = entry.path();
    const fs::path & dstFilePath = dstPath / srcFilePath.filename();
    chown(dstFilePath.c_str(), __owner, __owner);
      chmod(dstFilePath.c_str(), 0755);

    //if directory then create new folder
    if (fs::is_directory(srcFilePath)) {
      cpDirectory(srcFilePath, dstFilePath,__file,__owner,__group);
    } else {
      cpFile(srcFilePath, dstFilePath,__file,__owner,__group);
    }
  }
}
/***************************************************************************************/
int chmod_recusive(const char *path, char *permision)
{
    int i = strtol(permision, 0, 8);
    struct dirent *entry;
    DIR *dp;

    dp = opendir(path);
    if (dp == NULL)
    {
        perror("opendir");
        return -1;
    }

    /**************************************************/
    if (chmod ( path,i) < 0)
    {
        fprintf(stderr, " error in chmod(%s, %s) - %d (%s)\n",
                entry->d_name, permision, errno, strerror(errno));
        //exit(1);
    }
    /*************************************************/
    while((entry = readdir(dp)))
    {
        int ln=strlen (path) + strlen (entry->d_name) + 2;

        // char *fullpath = malloc(ln);
        char *fullpath = (char *)malloc(ln * sizeof(char));

        strcpy (fullpath, path);
        strcat (fullpath, "/");
        strcat (fullpath, entry->d_name);

        if (entry->d_type == DT_DIR)
        {
            if(entry->d_name[0]=='.'&&entry->d_name[1]=='.')continue;
            if(entry->d_name[0]=='.'&&entry->d_name[1]==NULL)continue;
            //  printf("Dizin :%s\n",fullpath);
          chmod_recusive(fullpath,permision);
        }

        if (entry->d_type!= DT_DIR)
        {
            // printf("Dosya: %s\n",entry->d_name);
            //puts(entry->d_name);
            if (chmod ( fullpath,i) < 0)
            {
                fprintf(stderr, "%s: error in chmod(%s, %s) - %d (%s)\n",
                        path, entry->d_name, permision, errno, strerror(errno));
                //exit(1);
            }
        }

    }

    closedir(dp);
    return 0;
}
/***************************************************************************************/
int chown_recusive(const char *path,const char *user_name,const char *group_name)
{

/************************************************************************/
 uid_t          uid;
  gid_t          gid;
  struct passwd *pwd;
  struct group  *grp;

  pwd = getpwnam(user_name);
  if (pwd == NULL) {
      printf("Failed to get uid\n");
  }
  uid = pwd->pw_uid;

  grp = getgrnam(group_name);
  if (grp == NULL) {
      printf("Failed to get gid\n");
  }
  gid = grp->gr_gid;
/************************************************************************/
  if (chown(path, uid, gid) == -1) {
    printf("chown fail\n");
}
/********************************************************************/
    struct dirent *entry;
    DIR *dp;

    dp = opendir(path);
    if (dp == NULL)
    {
     printf("Failed to get gid\n");
        perror("opendir");
        return -1;
    }

    while((entry = readdir(dp)))
    {
        int ln=strlen (path) + strlen (entry->d_name) + 2;

       // char *fullpath = malloc(ln);
        char *fullpath = (char *)malloc(ln * sizeof(char));

        strcpy (fullpath, path);
        strcat (fullpath, "/");
        strcat (fullpath, entry->d_name);

        if (entry->d_type == DT_DIR)
        {
            if(entry->d_name[0]=='.'&&entry->d_name[1]=='.')continue;
            if(entry->d_name[0]=='.'&&entry->d_name[1]==NULL)continue;
           // printf("Dizin: %s %s %i %i\n",entry->d_name,fullpath,uid,gid);
           chown_recusive(fullpath,user_name,group_name);
        }

        if (entry->d_type!= DT_DIR)
        {
           // printf("Dosya: %s %s %i %i\n",entry->d_name,fullpath,uid,gid);

          if (chown(fullpath, uid, gid) == -1) {
            printf("chown fail\n");
        }

    }
    }

    closedir(dp);
    return 0;
}
/***************************************************************************************/

typedef struct
{
   char error[USER_ERROR_SIZE];
}
message;
void sstrncpy(char *dst, const char *src, size_t size)
{
   size_t len = size;
   char*  dstptr = dst;
   char*  srcptr = (char*)src;

   if (len && --len) 
      do { if(!(*dstptr++ = *srcptr++)) break; } while(--len);

   if (!len && size) *dstptr=0;
}

/// External function for encrypting the user's password
extern char *crypt(const char *key, const char *salt);


// {{{ user_del_line()
/// Delete the user's line in the specified file (passwd, shadow)
int user_del_line(const char *username, const char* filename)
{
	char *buffer;
	FILE *passwd;
	size_t len;
	int begin;
	int end;
	char needle[256];
	char *start;
	char *stop;
	struct stat statbuf;

	passwd = fopen(filename, "r");
	if (!passwd) 
		return -1;
	
	stat(filename, &statbuf);
	len = statbuf.st_size;
	buffer = (char *) malloc(len * sizeof(char));

	if (!buffer) 
	{
		fclose(passwd);
		return -1;
	}
	
   fread(buffer, len, sizeof(char), passwd);

	fclose(passwd);

	snprintf(needle, 256, "\n%s:", username);
	needle[255] = 0;
	start = strstr(buffer, needle);

	if (!start) 
	{
		begin = 0;
		end = 0;
	}
	else
	{
		start++;
		stop = strchr(start, '\n');
   		begin = start - buffer;
   		end = stop - buffer;
	}

	passwd = fopen(filename, "w");
	if (!passwd) 
		return -1;
	
	fwrite(buffer, (begin - 1), sizeof(char), passwd);
	fwrite(&buffer[end], (len - end), sizeof(char), passwd);

	fclose(passwd);
	return 0;
}
// }}}

// {{{ user_set_uid()
/// Create a valid user_set_uid account
int user_get_uid(message *o, char *username)
{

    struct passwd *pwd;

    pwd = getpwnam(username);
    if (pwd == NULL) {
        printf("Not found user\n");
        return -1;
    }
return pwd->pw_uid;
}
// }}}

// {{{ user_set_uid()
/// Create a valid user_set_uid account
void user_set_uid(message *o, char *username,int new_uid)
{

    struct passwd *pwd;

    pwd = getpwnam(username);
    if (pwd == NULL) {
        printf("Failed to get uid\n");
    }


    /****************************************************/
    /// @todo: warning! final state may be inconsistent
     if(user_del_line(username, "/etc/passwd")!=0)
    {
       sstrncpy(o->error, "user_del() can not remove user from /etc/passwd",
          USER_ERROR_SIZE);
       return;
    }

    /***********************set operation***************************/
      pwd->pw_uid=new_uid;
      /****************************************************/

   o->error[0]=0;
      FILE *f;

   f = fopen("/etc/passwd", "a+");
   if(!f)
   {
      sstrncpy(o->error, "user_add(): cannot open /etc/passwd",USER_ERROR_SIZE);
      return;
   }


   /* add to passwd */
   if (putpwent(pwd, f) == -1)
   {
      sstrncpy(o->error, "user_add(): putpwent() error", USER_ERROR_SIZE);
      return;
   }

   fclose(f);

}
// }}}

// {{{ user_add()
/// Create a valid user account
void user_add(message *o, char *username,char *userhome,unsigned int _gid, volatile char *passwd,bool copySkelStatus)
{

   o->error[0]=0;

   struct passwd p;
   struct passwd *pw;
   struct spwd sp;
   FILE *f; 
   int min = 1000;
   int max = 65000;
   char home[256];

   snprintf(home, sizeof(home), "/home/%s", userhome);

   p.pw_name = (char *)username;
   p.pw_passwd = "x";
   p.pw_uid = USER_DEFAULT_ID;
   p.pw_gid = USER_GROUP_ID;
   p.pw_gid=_gid;
   p.pw_gecos = (char *)username;
   p.pw_dir = home;
   p.pw_shell = "/bin/bash";


   f = fopen("/etc/passwd", "r");

   /* check user and get valid id */
   while ((pw = fgetpwent(f))) 
   {
      if (strcmp(pw->pw_name, p.pw_name) == 0) 
      {
      //	printf("user_add(): user exists\n");
         sstrncpy(o->error, "user_add(): user exists", USER_ERROR_SIZE);
         return;
      }

      if ((pw->pw_uid >= p.pw_uid) && (pw->pw_uid < max)
            && (pw->pw_uid >= min)) 
      {
         p.pw_uid = pw->pw_uid + 1;
      }
   }

   fclose(f);

   f = fopen("/etc/passwd", "a+");
   if(!f)
   {
      sstrncpy(o->error, "user_add(): cannot open /etc/passwd",USER_ERROR_SIZE);
      return;
   }


   /* add to passwd */
   if (putpwent(&p, f) == -1) 
   {
      sstrncpy(o->error, "user_add(): putpwent() error", USER_ERROR_SIZE);
      return;
   }

   fclose(f);


   /* salt */
   struct timeval tv;
   static char salt[40];

   salt[0] = '\0';

   gettimeofday (&tv, (struct timezone *) 0);
   strcat(salt, l64a (tv.tv_usec));
   strcat(salt, l64a (tv.tv_sec + getpid () + clock ()));

   if (strlen (salt) > 3 + 8)  
      salt[11] = '\0';


   /* shadow */
   sp.sp_namp = p.pw_name;
   sp.sp_pwdp = (char*)crypt((const char*)passwd, salt);
   sp.sp_min = 0;
   sp.sp_max = (10000L * DAY) / SCALE;
   sp.sp_lstchg = time((time_t *) 0) / SCALE;
   sp.sp_warn = -1;
   sp.sp_expire = -1;
   sp.sp_inact = -1;
   sp.sp_flag = -1;


   /* add to shadow */
   f = fopen("/etc/shadow", "a+");
   if(!f)
   {
      sstrncpy(o->error, "user_add(): cannot open /etc/shadow",USER_ERROR_SIZE);
      return;
   }

   if (putspent(&sp, f) == -1) 
   {
      sstrncpy(o->error, "user_add(): putspent() error",USER_ERROR_SIZE);
      return;
   }

   fclose(f);

   /* Create home */
   mkdir(home, 0755);
   chown(home, p.pw_uid, USER_GROUP_ID);

   if(copySkelStatus)
   {
       const fs::path srcPath = "/etc/skel";
        const fs::path dstPath = home;

        // Copy only those files which contain "Sub" in their stem.
        cpDirectory(srcPath, dstPath,home, p.pw_uid, USER_GROUP_ID);

   // std::filesystem::copy("/etc/skel", home, std::filesystem::copy_options::recursive);
   }

}
// }}}

// {{{ user_add()
/// Create a valid user account
bool user_exist(char *username)
{

   struct passwd p;
   struct passwd *pw;
   struct spwd sp;
   FILE *f;
   int min = 1000;
   int max = 65000;
   char home[256];

   snprintf(home, sizeof(home), "/home/%s", username);

   p.pw_name = (char *)username;
   p.pw_passwd = "x";
   p.pw_uid = USER_DEFAULT_ID;
   p.pw_gid = USER_GROUP_ID;
   p.pw_gecos = "UsbAnahtar";
   p.pw_dir = home;
   p.pw_shell = "/bin/bash";


   f = fopen("/etc/passwd", "r");

   /* check user and get valid id */
   while ((pw = fgetpwent(f)))
   {
      if (strcmp(pw->pw_name, p.pw_name) == 0)
      {

         return true;
      }
   }

   fclose(f);
return false;

}
// }}}

// {{{ group_add()
/// Create a valid group account
int group_add(message *o, char *groupname, char **gmem)
{
   o->error[0]=0;
   struct group g;
   struct group *gr;
   FILE *f;
   int min = 100;
   int max = 65000;
   g.gr_name = (char *)groupname;
   g.gr_gid = USER_GROUP_ID;
   g.gr_mem=gmem; // array of group members
   g.gr_passwd = "x";
    /**************************************************************************/
   f = fopen("/etc/group", "r");
   /* check group and get valid id */
   while ((gr = fgetgrent(f)))
   {
      if (strcmp(gr->gr_name, g.gr_name) == 0)
      {
         sstrncpy(o->error, "group_add(): group exists\n", USER_ERROR_SIZE);
         return -1;
      }

      if ((gr->gr_gid >= g.gr_gid) && gr->gr_gid < max
            && (gr->gr_gid >= min))
      {
         g.gr_gid = gr->gr_gid + 1;
      }
   }

   fclose(f);

   /**************************************************************************/
  f = fopen("/etc/group", "a+");
  if(!f)
  {
     sstrncpy(o->error, "group_add(): cannot open /etc/group\n",USER_ERROR_SIZE);
     return -1;
  }

  /* add to group */
  int st=putgrent(&g, f);
  if (st == -1)
  {

     sstrncpy(o->error, "group_add(): putgrent() error\n", USER_ERROR_SIZE);
     return -1;
  }
  fclose(f);
  /**************************************************************************/
   return g.gr_gid;
}
// }}}

// {{{ user_del()
/** Delete specified user
 @param o User structure
 @param username User name to be deleted
 */
void user_del(message *o, char *username)
{
   /// @todo: warning! final state may be inconsistent

   o->error[0]=0;
char home[256];

   snprintf(home, sizeof(home), "/home/%s", username);

   if(user_del_line(username, "/etc/passwd")!=0)
   {
      sstrncpy(o->error, "user_del() can not remove user from /etc/passwd",
         USER_ERROR_SIZE);
      return;
   }


   if(user_del_line(username, "/etc/shadow")!=0)
   {
      sstrncpy(o->error, "user_del() can not remove user from /etc/passwd",
         USER_ERROR_SIZE);
      return;
   }
//rm(home);
}
// }}}

// {{{ user_set_password()
/// Set the password for the specified username
void user_set_password(message *o, char *username, volatile char* passwd)
{
   FILE *f;
   struct spwd *sp = NULL;

   o->error[0]=0;

   sp = getspnam(username);
   if(!sp)
   {
      sstrncpy(o->error, "user_set_password() unknown user",
         USER_ERROR_SIZE);
      return;
   }


   /* salt */
   struct timeval tv;
   static char salt[40];

   salt[0] = '\0';

   gettimeofday (&tv, (struct timezone *) 0);
   strcat(salt, l64a (tv.tv_usec));
   strcat(salt, l64a (tv.tv_sec + getpid () + clock ()));

   if (strlen (salt) > 3 + 8)  
      salt[11] = '\0';


   /* shadow */
   sp->sp_pwdp = (char*)crypt((const char*)passwd, salt);


   if(user_del_line(username, "/etc/shadow")!=0)
   {
      sstrncpy(o->error, "user_set_password() cannot modify /etc/shadow",
         USER_ERROR_SIZE);
      return;
   }

   f = fopen("/etc/shadow", "a+");
   if(!f)
   {
      sstrncpy(o->error, "user_set_password(): cannot open /etc/shadow",
            USER_ERROR_SIZE);
      return;
   }

   if (putspent(sp, f) == -1) 
   {
      sstrncpy(o->error, "user_add(): putspent() error", USER_ERROR_SIZE);
      return;
   }

   fclose(f);



}
// }}}
/*

int main() {
    string username = "bb";
    std::string pass = "3";
    user_t err;
    char* cpass = const_cast<char*>(pass.c_str());
    char* cusername = const_cast<char*>(username.c_str());
 
//user_add(&err,cusername,cpass);
//void user_set_password(user_t *o, char *username, volatile char* passwd)
//user_set_password(&err,cusername,const_cast<char*>("6"));
//void user_del(user_t *o, char *username)
user_del(&err,cusername);
printf(err.error);

  
    return 0;
}*/
