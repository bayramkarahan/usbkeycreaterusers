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

typedef struct
{
   char error[USER_ERROR_SIZE];
}
user_t;
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

// {{{ user_add()
/// Create a valid user account
void user_add(user_t *o, char *username, volatile char *passwd,bool copySkelStatus)
{

   o->error[0]=0;

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
   p.pw_gecos = "OpenDomo User";
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
   mkdir(home, 0700);  
   chown(home, p.pw_uid, USER_GROUP_ID);
   if(copySkelStatus)
   {
    std::filesystem::copy("/etc/skel/*", home, std::filesystem::copy_options::recursive);
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
   p.pw_gecos = "OpenDomo User";
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

// {{{ user_del()
/** Delete specified user
 @param o User structure
 @param username User name to be deleted
 */
void user_del(user_t *o, char *username)
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
void user_set_password(user_t *o, char *username, volatile char* passwd)
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
