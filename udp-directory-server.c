/******
* udp-directory-server.c
*
* Created on: Feb 23, 2016
* Author: Autumn K. Conner, Alisha Geis
* Course: CSE434, Project 1
******/

/***************DECLARATIONS***************/
#include <stdio.h> /* for printf() and fprintf() */
#include <sys/socket.h> /* for socket() and bind() */
#include <arpa/inet.h> /* for sockaddr_in and inet_ntoa() */
#include <stdlib.h> /* for atoi() and exit() */
#include <string.h> /* for memset() */
#include <unistd.h> /* for close() */
#define ECHOMAX 255 /* Longest string to echo */

int sock; /* Socket */
char responseBuffer[128]; /* holds the response to the client, max of 128 bytes */
struct sockaddr_in echoClntAddr; /* Client address */
void DieWithError(char *errorMessage); /* External error handling function */

/***************FUNCTIONS***************/
void OutputMsg(void) /* used to output the buffer to the client screen */
{
  int responseBufferSize = strlen(responseBuffer);

  /* Send contents of responseBuffer to the client */
  if (sendto(sock, responseBuffer, responseBufferSize, 0,
  (struct sockaddr *) &echoClntAddr, sizeof(echoClntAddr)) !=  responseBufferSize)
  DieWithError("sendto() sent a different number of bytes than expected");
}

/***************MAIN***************/
int main(int argc, char *argv[])
{
struct sockaddr_in echoServAddr; /* Local address */
//struct sockaddr_in echoClntAddr; /* Client address */
unsigned int cliAddrLen; /* Length of incoming message */
char echoBuffer[ECHOMAX]; /* Buffer for echo string */
char filedata[ECHOMAX]; /* Buffer for echo string */
unsigned short echoServPort; /* Server port */
int recvMsgSize; /* Size of received message */
struct message /* Struct to account for incoming messages from clients */
{ 
// total max bytes: 128     (30 + 30 + 20 + 48 = 128)
int messageType;
char hostName[30];
char address[30];  /* IP Address of Client */
char fileName[20]; /* Filename + directory path */
char fileSize[48];  /* Text File Max */
};

if (argc != 2) /* Test for correct number of parameters */
{
fprintf(stderr,"Usage: %s <UDP SERVER PORT>\n", argv[0]);
exit(1);
}

echoServPort = atoi(argv[1]); /* First arg: local port */
/* Create socket for sending/receiving datagrams */

if ((sock = socket(PF_INET, SOCK_DGRAM, IPPROTO_UDP)) < 0)
DieWithError("socket() failed");

/* Construct local address structure */
memset(&echoServAddr, 0, sizeof(echoServAddr)); /* Zero out structure */
echoServAddr.sin_family = AF_INET; /* Internet address family */
echoServAddr.sin_addr.s_addr = htonl(INADDR_ANY); /* Any incoming interface */
echoServAddr.sin_port = htons(echoServPort); /* Local port */
/* Bind to the local address */

if (bind(sock, (struct sockaddr *) &echoServAddr, sizeof(echoServAddr)) < 0)
DieWithError("bind() failed");

printf("DEBUG: OutputMsg %s\n", inet_ntoa(echoClntAddr.sin_addr));

for (;;) /* Run forever */
{
/* Set the size of the in-out parameter */
cliAddrLen = sizeof(echoClntAddr);

/* Block until receive message from a client */
if ((recvMsgSize = recvfrom(sock, echoBuffer, ECHOMAX, 0,
(struct sockaddr *) &echoClntAddr, &cliAddrLen)) < 0)
DieWithError("recvfrom() failed");
printf("Handling client %s\n", inet_ntoa(echoClntAddr.sin_addr));

char *mess = echoBuffer; //pointer to buffer to receive message in proper format
char* word;  /* point to next word in message */
char* field;  /* point to next field in listing entry */
const char s[2] = "/";
int i = 0;
int t;
long x;
char current[24];
struct message messArray[200]; //array of message structs for directory server to store


FILE *fptr;  /* fptr identifies a file to view or edit */
FILE *fptr2; /* fptr2 identifies a 2ND TEMPORARY file to edit */

char fname[40]; /* fname holds file name string information */
int filecount;  /* number of entries in CDS directory file */
char str1[128]; /* str1 is a temporary character array (a string) */
char str2[128]; /* str2 is a temporary character array (a string) */
int showFullList = 1;

strcpy(filedata, echoBuffer);

/* INSERT AND UPDATE */
if (echoBuffer[0] == '1') 
{
//OPEN A FILE FOR CREATION/APPENDING
  strcpy(fname, "./P2PFILES.txt");  /* the path and file name of the dir file */
	if ((fptr=fopen(fname,"a")) == NULL)
	{
		printf("Cannot open P2P Directory file %s \n",fname);
		//getch(); /* DEBUG */
	}
 	else
 	{
		fprintf(fptr,"%s\n", mess);
		fclose(fptr);
	}

//strcpy(mess, "HOSTNAME/ADDRESS/FILENAME/FILESIZE"); //DEBUG

word = strtok(mess, s);

word = strtok(NULL, s);
strcpy(messArray[i].hostName, word);
	printf("DEBUG: hostName %s\n", messArray[i].hostName);

word = strtok(NULL, s);
strcpy(messArray[i].address, word);
	printf("DEBUG: address %s\n", messArray[i].address);

word = strtok(NULL, s);
strcpy(messArray[i].fileName, word);
	printf("DEBUG: fileName %s\n", messArray[i].fileName);

word = strtok(NULL, "/ ");
strcpy(messArray[i].fileSize, word);
	printf("DEBUG: fileSize %s\n", messArray[i].fileSize);

strcpy(responseBuffer, "Inform and update option selected and completed.\n");
OutputMsg(); /* output the buffer to the client screen */

strcpy(messArray[i].hostName, current);
strcat(responseBuffer, current);
i++;
} /* end if echoBuffer[0] == '1'*/

/* QUERY */
else if(echoBuffer[0] == '2') 
{
if (echoBuffer[1] == '\0') showFullList = 1; /* no slash immediately following 2 */

else
{
showFullList = 0;
word = strtok(mess, s);   /* remove "2/" from the field */
if(word == NULL) printf("DEBUG: word is NULL\n");
word = strtok(NULL, s);  /* get filename between the slashes */
printf("DEBUG: word = %s\n", word);
}

if (showFullList == 1)
strcpy(responseBuffer, "Users in this community have these files:\n");
else
strcpy(responseBuffer, "Users in this community have this file:\n");
OutputMsg(); /* output the buffer to the client screen */

//OPEN FILE FOR READING
  strcpy(fname, "./P2PFILES.txt");  /* the path and file name of the dir file */
	if ((fptr=fopen(fname,"r")) == NULL)
	{
		printf("Can't open P2P Directory file %s \n",fname);
	}
 	else
 	{
		filecount = 0;
		while (fgets(str1, BUFSIZ, fptr) != NULL) /* Extract a line from the file */
		{
			if (showFullList == 1)
			{
			fputs(str1, stdout);  /* DEBUG Display that line to the server screen */
 			strcpy(responseBuffer, str1); /* copy one line to buffer for client to view */
			OutputMsg(); /* output the buffer to the client screen */
			}
			else
			{
				strcpy(str2, str1);
				field = strtok(str2, s); /* IP Address Field */
				field = strtok(NULL, s); /* File Name Field */
				
				if (strcmp(field,word) == 0)
				{
				fputs(str1, stdout);  /* DEBUG Display that line to the server screen */
				strcpy(responseBuffer, str1); /* copy one line to buffer for client to view */
				OutputMsg(); /* output the buffer to the client screen */
				}
			}

			filecount++;
		} /* end while */
		
		printf("DEBUG:Total entries in CDS directory file = '%d'\n", filecount); /**/
		fclose(fptr);
		
	} /* end if */
} /* end else if echoBuffer[0] == '2'*/

/* EXIT */
else if(echoBuffer[0] == '3') 
{
// open the directory file and look for data corresponding to the current P2P client
// delete any directory entries for that client from the file
strcpy(responseBuffer, "Exit Command Received");
OutputMsg();

//OPEN A TEMPORARY FILE FOR WRITING
  strcpy(fname, "./P2PTEMP.txt");  /* the path and file name of the dir file */
	if ((fptr2=fopen(fname,"w")) == NULL)
	{
		printf("Can't open P2P Temporary file %s \n",fname);
	}

//OPEN FILE FOR READING
  strcpy(fname, "./P2PFILES.txt");  /* the path and file name of the dir file */
	if ((fptr=fopen(fname,"r")) == NULL)
	{
		printf("Can't open P2P Directory file %s \n",fname);
	}
 	else
 	{
		filecount = 0;
		//printf("DEBUG:fopen file name is '%s'\n", fname); /**/
		while (fgets(str1, BUFSIZ, fptr) != NULL) /* Extract a line from the file */
		{
			strcpy(str2, str1);
			field = strtok(str2, s); /* IP Address Field */
			
			if (strcmp(inet_ntoa(echoClntAddr.sin_addr),field) != 0)
			{
				fputs(str1, stdout);  /* DEBUG Display that line to the server screen */
				fprintf(fptr2,"%s", str1);
			}
			filecount++;
		} /* end while */

		printf("DEBUG:Total entries in CDS directory file = '%d'\n", filecount); /**/
		fclose(fptr);
		fclose(fptr2);
    } /* end else */

//OPEN ORIGINAL FILE FOR WRITING
  strcpy(fname, "./P2PFILES.txt");  /* the path and file name of the dir file */
	if ((fptr=fopen(fname,"w")) == NULL)
	{
		printf("Can't open P2P Directory file %s \n",fname);
	}

//OPEN THE TEMPORARY FILE FOR READING
  strcpy(fname, "./P2PTEMP.txt");  /* the path and file name of the dir file */
	if ((fptr2=fopen(fname,"r")) == NULL)
	{
		printf("Can't open P2P Temporary file %s \n",fname);
	}
		while (fgets(str1, BUFSIZ, fptr2) != NULL) /* Extract a line from the file */
		{
			fputs(str1, stdout);  /* DEBUG Display that line to the server screen */
			fprintf(fptr,"%s", str1);

		} /* end while */

		fclose(fptr);
		fclose(fptr2);
		remove(fname);

} /* end else if echoBuffer[0] == '3'*/

else 
{
strcpy(responseBuffer, "Invalid command received");
OutputMsg();
}

}
/* NOT REACHED */
}
