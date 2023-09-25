#include <QCoreApplication>

#include <libudev.h>
#include <stdio.h>
#include <stdlib.h>
#include <locale.h>
#include <unistd.h>
#include <string.h>
#include <iostream>
#include <stdio.h>
#include <sys/statvfs.h>
#include <string_view>
#include <optional>
#include <cstring>
#include <fstream>
#include <string>
#include <iostream>
#include <sstream>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <openssl/md5.h>
#include <iostream>
#include <sstream>
#include <vector>
#include<user.h>
#include <curl/curl.h>

// g++ udev-mountslist -ludev -lcrypto -lssl

using namespace std;
bool disk_add_drm=false;
bool disk_remove_drm=false;
std::string login_serial="";
bool login_status=false;
static
struct udev_device*
get_child(struct udev* udev, struct udev_device* parent, const char* subsystem) {
  struct udev_device* child = NULL;
  struct udev_enumerate *enumerate = udev_enumerate_new(udev);

  udev_enumerate_add_match_parent(enumerate, parent);
  udev_enumerate_add_match_subsystem(enumerate, subsystem);
  udev_enumerate_scan_devices(enumerate);

  struct udev_list_entry *devices = udev_enumerate_get_list_entry(enumerate);
  struct udev_list_entry *entry;

  udev_list_entry_foreach(entry, devices) {
      const char *path = udev_list_entry_get_name(entry);
      child = udev_device_new_from_syspath(udev, path);
      break;
  }

  udev_enumerate_unref(enumerate);
  return child;
}

static void enumerate_usb_mass_storage(struct udev* udev) {
  struct udev_enumerate* enumerate = udev_enumerate_new(udev);

  udev_enumerate_add_match_subsystem(enumerate, "scsi");
  udev_enumerate_add_match_property(enumerate, "DEVTYPE", "scsi_device");
  udev_enumerate_scan_devices(enumerate);

  struct udev_list_entry *devices = udev_enumerate_get_list_entry(enumerate);
  struct udev_list_entry *entry;

  udev_list_entry_foreach(entry, devices) {
      const char* path = udev_list_entry_get_name(entry);
      struct udev_device* scsi = udev_device_new_from_syspath(udev, path);

      struct udev_device* block = get_child(udev, scsi, "block");
      struct udev_device* scsi_disk = get_child(udev, scsi, "scsi_disk");

      struct udev_device* usb
          = udev_device_get_parent_with_subsystem_devtype(scsi, "usb", "usb_device");

      if (block && scsi_disk && usb) {
          printf("block = %s, usb = %s:%s, scsi = %s, name = %s, size = %s, x = %i, x = %s, x = %s\n",
                 udev_device_get_devnode(block),
                 udev_device_get_sysattr_value(usb, "idVendor"),
                 udev_device_get_sysattr_value(usb, "idProduct"),
                 udev_device_get_sysattr_value(scsi, "vendor"),
udev_device_get_sysattr_value(usb, "udev"),
udev_device_get_sysattr_value(usb, "udev_root"),
udev_device_get_sysattr_value(usb, "size"),
udev_device_get_sysattr_value(usb, "speed"),
udev_device_get_sysattr_value(usb, "bMaxPower"));

      }

      if (block) {
          udev_device_unref(block);
      }

      if (scsi_disk) {
          udev_device_unref(scsi_disk);
      }
      udev_device_unref(scsi);
  }

  udev_enumerate_unref(enumerate);
}

/*************************************mounts list**************************************/
std::optional<std::string> get_device_of_mount_point(std::string_view devicepath)
{
   std::ifstream mounts{"/proc/mounts"};
   std::string device, mountpoint, type;
   std::string line;


if (mounts.is_open()) {
    while (std::getline(mounts, line)) {

    std::istringstream iss(line);

    getline(iss, device, ' ');
    getline(iss, mountpoint, ' ');
    getline(iss, type, ' ');
         if (device == devicepath)
         {
            //std::cout << device << '|'<< mountpoint << '|'<< type<< std::endl;
            return mountpoint;
         }
    }
    mounts.close();
}

  return std::nullopt;
}
/******************************************************************************************/
std::vector<std::string> split (const std::string &s, char delim) {
    std::vector<std::string> result;
    std::stringstream ss (s);
    std::string item;

    while (getline (ss, item, delim)) {
        result.push_back (item);
    }

    return result;
}
/*************************************md5 cmp**************************************/
std::string  exist_md5(std::string mountpoint,std::string md5kod)
{
mountpoint.append("/.ebaqr");

   std::ifstream file(mountpoint);
    std::string line;


if (file.is_open()) {
    while (std::getline(file, line)) {

    std::istringstream iss(line);
    const auto satir=std::string(line);

  //  getline(iss, device, ' ');
  //  getline(iss, mountpoint, ' ');
  //  getline(iss, type, ' ');
    // std::cout << "bilgiler!" <<line<<md5kod<<'\n';
    if (satir.find(md5kod) != std::string::npos) {
        //std::cout << "Bulundu!" <<line<<'\n';
        return line;
    }

       /*  if (line== md5kod)
         {
            //std::cout << "kod var açılabilir "<< std::endl;
            return true;
         }*/

    }
    file.close();

}
return "";

}
/******************************************************************************************/
/*************************************qrpsw cmp**************************************/
std::string get_qrpsw()
{

   std::ifstream file("/etc/qrpsw");
    std::string line;


if (file.is_open()) {
    while (std::getline(file, line)) {

                return line;

    }
    file.close();
}

  return "";
}
/******************************************************************************************/
/*************************************get_umountdisk_command cmp**************************************/
std::string get_umountdisk_command()
{

   std::ifstream file("/usr/share/usbkeycreater/command.conf");
    std::string line;


if (file.is_open()) {
    while (std::getline(file, line)) {

                return line;

    }
    file.close();
}

  return "";
}
/******************************************************************************************/
/****************************************md5sum********************************************/
char *str2md5(const char *str, int length) {
    int n;
    MD5_CTX c;
    unsigned char digest[16];
    char *out = (char*)malloc(33);

    MD5_Init(&c);

    while (length > 0) {
        if (length > 512) {
            MD5_Update(&c, str, 512);
        } else {
            MD5_Update(&c, str, length);
        }
        length -= 512;
        str += 512;
    }

    MD5_Final(digest, &c);

    for (n = 0; n < 16; ++n) {
        snprintf(&(out[n*2]), 16*2, "%02x", (unsigned int)digest[n]);
    }

    return out;
}
/*******************************************************************************************/
char *subString(const char *s, int index, int n) {

    char *res = (char*)malloc(n );
    if (res) {
        strncpy(res, s + index, n );
    }
    return res;
}
/******************************************************************************************/
void user_add_two(std::string *username, std::string *passwd)
{
    message err;
    char* cpass = const_cast<char*>(passwd->c_str());
    char* cusername = const_cast<char*>(username->c_str());

    char* cpassqr = const_cast<char*>(username->c_str());
    std::string usernameqr;    usernameqr.append(username->c_str()); usernameqr.append("-qr");
    char* cusernameqr = const_cast<char*>(usernameqr.c_str());
   // printf("kallanıcı 1 %s şifre: %s\n",cusername,cpass);
   // printf("kallanıcı 1 %s şifre: %s\n",cusernameqr,cpassqr);

    /*****************************************************************/
    char *mems[]={cusername,cusernameqr,NULL};
    int _gid=group_add(&err, cusername, mems);
    user_add(&err,cusername,cusername,_gid,cusername,true);//user
    user_add(&err,cusernameqr,cusername,_gid,cpass,false);//user-qr

    char home[256];
    snprintf(home, sizeof(home), "/home/%s", cusername);
    chown_recusive(home,cusername,cusername); //chown $user -R /home/$user
    chmod_recusive(home,"755");// chmod 755 /home/$user
    int a=user_get_uid(&err,cusername); //uida=$(grep "^$user:" /etc/passwd | cut -f 3 -d ":")
   // int b=user_get_uid(&err,cusernameqr); //uidb=$(grep "^$user-qr:" /etc/passwd | cut -f 3 -d ":")
    user_set_uid(&err,cusernameqr,a);// sed -i "s/:$uidb:/:$uida:/g" /etc/passwd
   // for g in floppy audio video plugdev netdev $user; do usermod -aG $g $user-qr || true;usermod -aG $g $user || true;done

    std::string kmt="for g in cdrom floppy audio dip video plugdev netdev bluetooth lpadmin scanner ";
    kmt.append(cusername);
    kmt.append("; do usermod -aG $g ");
    kmt.append(cusernameqr);
    kmt.append(" || true;usermod -aG $g ");
    kmt.append(cusername);
    kmt.append(" || true;done &");
    system(kmt.c_str());
   // printf("%s\n",kmt.c_str());


    //
    /*
     //
     std::string kmt="addgroup ";
     kmt.append(username);
     system(kmt.c_str());

     user_add(&err,cusername,md5pass,true);
     // system("loginctl terminate-seat seat0");

     std::string kmtg="chgrp ";
     kmtg.append(username);
     kmtg.append(" -R /home/");
     kmtg.append(username);
     system(kmtg.c_str());
*/
}
int login_curl(std::string data)
{
  CURL *curl;
  CURLcode res;
  CURLoption co;
 // std::string data;
 //    printf("curl data: %s\n",data.c_str());
  /* In windows, this will init the winsock stuff */
  curl_global_init(CURL_GLOBAL_ALL);

  /* get a curl handle */
  curl = curl_easy_init();
  if(curl) {
    /* First set the URL that is about to receive our POST. This URL can
       just as well be an https:// URL if that is what should receive the
       data. */
    curl_easy_setopt(curl, CURLOPT_URL, "http://127.0.0.1:8080");
    /* Now specify the POST data */
    curl_easy_setopt(curl, CURLOPT_POSTFIELDS,data.c_str());

    /* Perform the request, res will get the return code */
    res = curl_easy_perform(curl);
    /* Check for errors */
    if(res != CURLE_OK)
      fprintf(stderr, "curl_easy_perform() failed: %s\n",
              curl_easy_strerror(res));

    /* always cleanup */
    curl_easy_cleanup(curl);
  }
  curl_global_cleanup();
  return 0;
}

void login_process(udev_device *dev)
{
    printf("Disk Bilgisi Okunuyor...\n");
    disk_add_drm=false;
    auto sserial=udev_device_get_property_value(dev, "ID_SERIAL_SHORT");
    /********************************md5*****************************************************/
    auto md5kod = str2md5(sserial, strlen(sserial));
    auto pass=subString(sserial,0,6);
    // printf("pass Bilgi: %s",pass);
    auto temppassmd5=str2md5(pass, strlen(pass));
    char *md5pass=subString(temppassmd5,0,6);
    //printf("md5pass Bilgi: %s",md5pass);
    /************disk bağlantı noktası öğrenme erişme kod kontrolü**************/
    if (const auto point = get_device_of_mount_point(udev_device_get_devnode(dev)))
    {
        std::cout <<"point: "<< *point << "\n";
        std::vector<std::string> diskbilgilist;
        std::string  diskbilgi=exist_md5(*point,md5kod);//disk içindeki md5 kontrol ediliyor
       // printf("disk Bilgi: %s\n",diskbilgi.c_str());

        if (diskbilgi.length()>0)
        {
            diskbilgilist = split (diskbilgi, '|');
            // printf("Dosya İçeriği Bilgisi: %s %i",diskbilgi.c_str(),diskbilgilist.size());
            //burada hash ve username konttrolü yapılacak......
            //......................................................
            //......................................................

            if(diskbilgilist.size()<3) return;

        }else
        {
            printf("Dosya Bulunamadı *****..\n");
            return;
        }

        std::string kod=diskbilgilist.at(0);
        std::string username=diskbilgilist.at(1);
        auto tempusernamemd5=str2md5(username.c_str(), strlen(username.c_str()));
        std::string usernamehash=diskbilgilist.at(2);

        if( strcmp(tempusernamemd5,usernamehash.c_str())==0)
            printf("kullanıcı bilgisi Doğru\n");
        else
        {
            printf("kullanıcı bilgisi hatalı\n");
            return;
        }

        // printf("tempusernamehash: %s\n",tempusernamemd5);
        // printf("usernamehash: %s\n",usernamehash.c_str());

        printf("İşlemler başlıyor..\n");
        //printf("Okunan Disk Bilgisi kod: %s\n",diskbilgilist.at(0).c_str());
        //printf("Okunan Disk Bilgisi username: %s\n",diskbilgilist.at(1).c_str());

        if(kod==md5kod)
        {

            printf("Disk Kodu ve Dosyaki Kod Aynı Sorun yok\n");

            char* un =strdup(username.c_str());

            if(user_exist(un))
            {
                printf("Kullanıcı sistemde var\n");
                printf("username:%s\n",username.c_str());
                std::string seri=sserial;
                std::string id = seri.substr (0,6);
                // printf("id:%s\n",id.c_str());
                //char* cpass = const_cast<char*>(id.c_str());
                char* cusername = const_cast<char*>(username.c_str());
               // std::string komut="/usr/bin/sshlogin ";
                std::string curldata="username=";
                if(username=="ebaqr")
                {
                    std::string qrpsw=get_qrpsw();
                   /* komut.append("ebaqr ");
                    komut.append(qrpsw);*/
                    curldata.append("ebaqr&password=");
                    curldata.append(qrpsw);
                }
                else
                {
                   /* komut.append(cusername);
                    komut.append("-qr");
                    komut.append(" ");
                    komut.append(md5pass);*/
                    curldata.append(cusername);
                    curldata.append("-qr&password=");
                    curldata.append(md5pass);


                }
                //komut.append(" &");
                //printf("açılış komutu: %s \n",komut.c_str());
                //printf("açılış komutu: %s \n",curldata.c_str());

                //system(komut.c_str());

              login_curl(curldata);
            }
            else
            {
                printf("Kullanıcı sistemde yok\n");
                printf("username:%s\n",username.c_str());
                std::string seri=sserial;
                std::string id = seri.substr (0,6);
                printf("id:%s\n",id.c_str());
                //char* cpass = const_cast<char*>(md5pass.c_str());
                char* cusername = const_cast<char*>(username.c_str());

                std::string pss;
                pss.append(md5pass);

                user_add_two(&username,&pss);
                system("systemctl restart lightdm.service");

                /*****************şifre sormasın diye dosya oluşturuluyor************/
                std::string yol="/home/"+username+"/.config/np";
                std::ofstream o(yol.c_str());
                o << "\n" << std::endl;

                /*******************************************************************/
                /*std::string komut="/usr/bin/sshlogin ";
                komut.append(cusername);
                 komut.append("-qr");
                komut.append(" ");
                komut.append(md5pass);
                komut.append(" &");
                printf("açılış komutu: %s \n",komut.c_str());
                system(komut.c_str());*/
                std::string curldata="username=";
                curldata.append(cusername);
                curldata.append("-qr&password=");
                curldata.append(md5pass);

                 login_curl(curldata);

            }

            login_status=true;
            login_serial=sserial;


        }

////işlem bittikten sonra  umount yapılıyor
///

    }
    /****************************************************************************************************/

}
/******************************************************************************************/
int main(int argc, char *argv[]){

    QCoreApplication a(argc, argv);



    struct udev* udev = udev_new();
    enumerate_usb_mass_storage(udev);


    struct udev_device *dev;
    struct udev_monitor *mon;
    int fd;

    mon = udev_monitor_new_from_netlink(udev, "udev");
    udev_monitor_filter_add_match_subsystem_devtype(mon, "block", NULL);
    udev_monitor_enable_receiving(mon);
    fd = udev_monitor_get_fd(mon);

    while (1) {

        fd_set fds;
        struct timeval tv;
        int ret;

        FD_ZERO(&fds);
        FD_SET(fd, &fds);
        tv.tv_sec = 0;
        tv.tv_usec = 0;

        ret = select(fd+1, &fds, NULL, NULL, &tv);

        /* Check if our file descriptor has received data. */
        if (ret > 0 && FD_ISSET(fd, &fds))
        {
            printf("\nselect() says there should be data\n");

            dev = udev_monitor_receive_device(mon);
            if (dev) {
                printf("Got Device\n");
                printf("   Node: %s\n", udev_device_get_devnode(dev));
                printf("   Subsystem: %s\n", udev_device_get_subsystem(dev));
                printf("   Devtype: %s\n", udev_device_get_devtype(dev));
                printf("   Action: %s\n", udev_device_get_action(dev));
                ///cout << udev_get_run_path (dev) << endl;

                if( strcmp(udev_device_get_action(dev),"add")==0 && strcmp(udev_device_get_devtype(dev),"partition")==0)
                {
                    disk_add_drm=true;
                    cout << "conectat" << endl;
                    //tmp/addusb
                    system("umount /media/usbkeydisk");

                    system("mkdir /media/usbkeydisk");

                    std::string komut="mount ";
                    komut.append(udev_device_get_devnode(dev));
                    komut.append(" /media/usbkeydisk");

                    printf("komut: %s\n",komut.c_str());
                    //std::cout <<"komut:"<<komut<< "\n";

                    system(komut.c_str());
                    printf("disk bağlandı..\n");
                }
                if( strcmp(udev_device_get_action(dev),"remove")==0 )
                {
                    disk_remove_drm=true;
                    auto sserial=udev_device_get_property_value(dev, "ID_SERIAL_SHORT");
                    //login olunmuş ve login olan disk ile çıkartılan disk aynıysa logout yap
                    std::string kmt="umount ";
                        kmt.append(udev_device_get_devnode(dev));
                        system(kmt.c_str());
                        //system("systemctl restart usb-mount.service|true");
                      if(sserial==login_serial&&login_status)
                    {
                        login_serial="";
                        login_status=false;
                        disk_add_drm=false;
                        printf("oturum açılan disk umount yapıldı");
                        //usb disk çıkartıldığında çalışacak komut çalıştırılıyor...
            //system("loginctl terminate-seat seat0");

                        printf("-----------------------------------------------------------------\n");
                        printf("remove umount yapıldı\n");
                        printf("-----------------------------------------------------------------\n");
                        system("umount /media/usbkeydisk&");

            system(get_umountdisk_command().c_str());

                    }

                }
                /********************************disk bağlandımı*****************************/
                if( strcmp(udev_device_get_action(dev),"add")==0&&(disk_add_drm==true&&login_status==false) )
                {
                    login_process(dev);

                    printf("-----------------------------------------------------------------\n");
                    printf("add umount yapıldı\n");
                    printf("-----------------------------------------------------------------\n");
                    system("umount /media/usbkeydisk&");

                }

                if( strcmp(udev_device_get_action(dev),"change")==0 && strcmp(udev_device_get_devtype(dev),"partition")==0)
                {

                    printf("-----------------------------------------------------------------\n");
                    printf("change umount yapıldı\n");
                    printf("-----------------------------------------------------------------\n");
                    system("umount /media/usbkeydisk&");

                }
                udev_device_unref(dev);
            }
            else {
                printf("No Device from receive_device(). An error occured.\n");
            }

        }
        //printf("Usb taranıyor...\n");

        usleep(250*1000);
        fflush(stdout);
    }




    udev_unref(udev);

     return a.exec();
    //return 0;
}
/*int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);

    return a.exec();
}*/

