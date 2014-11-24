/*
 *  tcpsocket.c
 *  LIME, The versatile 3D line modeling environment
 *
 *  Created by Christian Brinch on 01/02/13.
 *  Copyright 2006-2014, Christian Brinch,
 *  <brinch@nbi.dk>
 *  Niels Bohr institutet
 *  University of Copenhagen
 *	All rights reserved.
 *
 */

#include "lime.h"
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netdb.h>

#define USERAGENT "HTMLGET 1.0"
int close(int);

void
openSocket(inputPars *par, int id){
  struct sockaddr_in *remote;
  int tmpres;
  int sock;
  char *get;
  char *s,*t;
  char buf[2];
  char *host = "home.strw.leidenuniv.nl";
  char *ip="132.229.214.164";
  char *page = "~moldata/datafiles/";
  char *tpl= "GET /%s HTTP/1.0\r\nHost: %s\r\nUser-Agent: %s\r\n\r\n";
  FILE *fp;

  // Check if moldatfile contains .dat
  if(strstr(par->moldatfile[id], ".dat") == NULL){
    strcat( par->moldatfile[id],  ".dat" );
  }

  size_t length1 = strlen(page);
  size_t length2 = strlen(par->moldatfile[id]);
  t = (char*)malloc(sizeof(char) * (length1 + length2 + 1));
  strcpy(t,page);
  strcat(t, par->moldatfile[id]);
  t[length1+length2+1]='\0';
  page=t;


  // Create socket (similar to open file)
  if((sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP)) < 0){
    if(!silent) bail_out("Can't create TCP socket");
    exit(1);
  }

  remote = (struct sockaddr_in *)malloc(sizeof(struct sockaddr_in *));
  remote->sin_family = AF_INET;
  tmpres = inet_pton(AF_INET, ip, (void *)(&(remote->sin_addr.s_addr)));
  if( tmpres < 0)
    {
      if(!silent) bail_out("Can't set remote->sin_addr.s_addr");
      exit(1);
    }else if(tmpres == 0)
      {
        if(!silent) bail_out("Not a valid IP address");
        exit(1);
      }
    remote->sin_port = htons(80);

    if(connect(sock, (struct sockaddr *)remote, sizeof(struct sockaddr)) < 0){
      if(!silent) bail_out("Could not connect");
      exit(1);
    }

    // -5 is to consider the %s %s %s in tpl and the ending \0
    get = (char *)malloc(strlen(host)+strlen(page)+strlen(USERAGENT)+strlen(tpl)-5);
    sprintf(get, tpl, page, host, USERAGENT);

    //fprintf(stderr, "Query is:\n<<START>>\n%s<<END>>\n", get);

    //Send the query to the server
    int sent = 0;
    while(sent < strlen(get))
      {
        tmpres = send(sock, get+sent, strlen(get)-sent, 0);
        if(tmpres == -1){
          perror("Can't send query");
          exit(1);
        }
        sent += tmpres;
      }

    memset(buf, 0, sizeof(buf));
    if((fp=fopen(par->moldatfile[id], "w"))==NULL) {
      if(!silent) bail_out("Failed to write moldata!");
      exit(1);
    }

    int flag=0;
    while((tmpres = recv(sock, buf, 1, 0)) > 0){
      if(strstr(buf,"!")) flag=1;
      if(flag) fprintf(fp,"%s",buf);
    }

    free(get);
    free(remote);
    close(sock);
    fclose(fp);
}
