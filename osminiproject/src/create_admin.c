#include "../header/head.h" // Include necessary header file

int main()
{
    char buff[100];
    memset(buff,0,sizeof(buff)); // Initialize buffer with zeros
    printf("Enter Adminname: => ");
    scanf("%s",buff); // Read admin username from user input

    // Create a User struct to store admin credentials
    struct User u;
    strcpy(u.username,buff); // Copy admin username to User struct
    printf("Enter password: => ");
    memset(buff,0,sizeof(buff)); // Clear buffer
    scanf("%s",buff); // Read admin password from user input
    strcpy(u.password,buff); // Copy admin password to User struct
    u.type=0; // Set user type as admin

    // Open users database file
    int fd=open("../db/users.bin",O_CREAT|O_RDWR,0666);
    lseek(fd,0,SEEK_END); // Move file pointer to the end of the file
    write(fd,&u,sizeof(u)); // Write admin credentials to the file
    close(fd); // Close file descriptor
}
