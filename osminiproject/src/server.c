#include "../header/server.h"
#include "../header/head.h"

sem_t books_semaphore;
sem_t users_semaphore;
sem_t issues_semaphore;

// Function to get the next available book ID
int get_id(){
    int fd=open("../db/books.bin",O_RDONLY,0666);
    int offset=lseek(fd,0,SEEK_END);
    if(offset <= 0) return 1;
    else return(offset/sizeof(struct Book) + 1);
}

// Function to display all books stored in the database
void get_all_books(){
    struct Book temp;
    int fd=open("../db/books.bin",O_RDONLY,0666);
    read(fd,&temp,sizeof(struct Book));
    printf("Title: %s\n",temp.title);
    printf("Author: %s\n",temp.author);
    printf("Copies: %d\n",temp.copies);
    printf("Valid: %d\n",temp.valid);
    printf("ID: %d\n",temp.id);
    printf("\n");
    while(read(fd,&temp,sizeof(struct Book)))
    {
        printf("Title: %s\n",temp.title);
        printf("Author: %s\n",temp.author);
        printf("Copies: %d\n",temp.copies);
        printf("Valid: %d\n",temp.valid);
        printf("ID: %d\n",temp.id);
        printf("\n");
    }
}

// Function to add a new book to the database
void add_book(int nsd){ // here nsd is like socket descriptor
    // Wait for semaphore to ensure exclusive access to shared resource
    sem_wait(&books_semaphore);

    // Declare and initialize a temporary Book structure
    struct Book temp;
    bzero(&temp,sizeof(struct Book)); // memset can also be used

    // Read the book details sent by the client
    read(nsd,&temp,sizeof(struct Book));
    
    // Get the ID for the new book
    temp.id = get_id();

    // Open the books database file
    int fd=open("../db/books.bin",O_WRONLY|O_CREAT,0666);
    
    // Move the file pointer to the end of the file
    lseek(fd,0,SEEK_END);

    // Write the new book details to the database
    write(fd,&temp,sizeof(struct Book));
    
    // Close the database file
    close(fd);

    // Send all books to the client after adding the new book
    get_all_books();

    // Release semaphore to allow other threads to access shared resource
    sem_post(&books_semaphore);
}

// Function to send all books to the client
void send_books(int nsd){
    sem_wait(&books_semaphore);

    int fd=open("../db/books.bin",O_RDONLY,0666);
    int offset=lseek(fd,0,SEEK_END);

    int cnt = -1;
    if(offset <= 0) cnt = 0;
    else cnt = (offset/sizeof(struct Book));
    printf("%d\n",cnt);
    write(nsd,&cnt,sizeof(int));

    struct Book temp;
    for(int i=0;i<cnt;i++){
        lseek(fd,i*sizeof(struct Book),SEEK_SET);
        read(fd,&temp,sizeof(struct Book));
        write(nsd,&temp,sizeof(struct Book));
    }
    close(fd);

    sem_post(&books_semaphore);
}

// Function to delete a book from the database
void delete_book(int nsd){
    sem_wait(&books_semaphore);

    int torid = -1;
    read(nsd,&torid,sizeof(int));

    struct Book temp;
    int fd=open("../db/books.bin",O_WRONLY,0666);


    lseek(fd,(torid-1)*sizeof(struct Book),SEEK_SET);
    read(fd,&temp,sizeof(struct Book));
    temp.valid = 0;
    strcpy(temp.title,"");
    strcpy(temp.author,"");
    temp.id = -1;
    temp.copies = -1;
    lseek(fd,(torid-1)*sizeof(struct Book),SEEK_SET);
    write(fd,&temp,sizeof(struct Book));
    
    get_all_books();

    sem_post(&books_semaphore);
}

// Function to modify a book's information in the database
void modify_book(int nsd){
    sem_wait(&books_semaphore);

    int cnid = -1, cncp = -1;
    read(nsd,&cnid,sizeof(int));
    read(nsd,&cncp,sizeof(int));

    struct Book temp;
    int fd=open("../db/books.bin",O_RDWR,0666);

    lseek(fd,(cnid-1)*sizeof(struct Book),SEEK_SET);
    read(fd,&temp,sizeof(struct Book));

    temp.copies = cncp;

    lseek(fd,(cnid-1)*sizeof(struct Book),SEEK_SET);
    write(fd,&temp,sizeof(struct Book));
    
    get_all_books();
    sem_post(&books_semaphore);
}

// Function to add a new user to the database
void add_user(int nsd){
    sem_wait(&users_semaphore);
    struct User u;
    read(nsd,&u,sizeof(struct User));

    int fd=open("../db/users.bin",O_CREAT|O_RDWR,0666);
    lseek(fd,0,SEEK_END);
    write(fd,&u,sizeof(u));
    close(fd);
    sem_post(&users_semaphore);
}

// Function to send all users to the client
void send_users(int nsd){
    sem_wait(&users_semaphore);
    int fd = open("../db/users.bin",O_RDWR,0666);
    int cnt = lseek(fd,0,SEEK_END)/sizeof(struct User);

    write(nsd,&cnt,sizeof(int));
    struct User temp;

    for(int i=0;i<cnt;i++){
        lseek(fd,i*sizeof(struct User),SEEK_SET);
        read(fd,&temp,sizeof(struct User));
        write(nsd,&temp,sizeof(struct User));
    }
    close(fd);
    sem_post(&users_semaphore);
}

// Function to display all issued books
void get_all_issues(){

    struct issue temp;
    int fd=open("../db/issues.bin",O_RDONLY,0666);
    read(fd,&temp,sizeof(struct issue));
    printf("bookid: %d\n",temp.bookid);
    printf("username: %s\n",temp.username);
    printf("valid: %d\n",temp.valid);
    printf("\n");
    while(read(fd,&temp,sizeof(struct issue)))
    {
        printf("bookid: %d\n",temp.bookid);
        printf("username: %s\n",temp.username);
        printf("valid: %d\n",temp.valid);
        printf("\n");
    }
}

// Function to issue a book to a user
void issue_book(int nsd){
    sem_wait(&books_semaphore);
    sem_wait(&issues_semaphore);

    struct issue temp;
    read(nsd,&temp,sizeof(struct issue));

    struct Book newbook;
    int fd=open("../db/books.bin",O_RDWR,0666);

    lseek(fd,(temp.bookid-1)*sizeof(struct Book),SEEK_SET);
    read(fd,&newbook,sizeof(struct Book));
    if(newbook.copies<=0 || newbook.valid == 0){
        int status = -1;
        write(nsd,&status,sizeof(int));
        sem_post(&books_semaphore);
        sem_post(&issues_semaphore);
        return;
    }

    newbook.copies = newbook.copies -1;

    lseek(fd,(temp.bookid-1)*sizeof(struct Book),SEEK_SET);
    write(fd,&newbook,sizeof(struct Book));

    close(fd);


    fd=open("../db/issues.bin",O_WRONLY|O_CREAT,0666);

    lseek(fd,0,SEEK_END);
    write(fd,&temp,sizeof(struct issue));
    close(fd);

    int status = 1;
    write(nsd,&status,sizeof(int));  

    get_all_issues();

    sem_post(&issues_semaphore);
    sem_post(&books_semaphore);
}

// Function to return a book to the library
void return_book(int nsd){

    sem_wait(&books_semaphore);
    sem_wait(&issues_semaphore);
        struct issue temp;
    read(nsd,&temp,sizeof(struct issue));
    temp.valid = false;
    bool flag = false;

    struct issue tempissue;
    int fd=open("../db/issues.bin",O_RDWR,0666);
    lseek(fd,0,SEEK_SET);
    while(read(fd,&tempissue,sizeof(struct issue))>0){
        printf("%d",tempissue.bookid);
        printf(" %s\n",tempissue.username);
        if(tempissue.bookid == temp.bookid && !strcmp(temp.username,tempissue.username) && tempissue.valid!=0){
            flag = true;
            lseek(fd,-sizeof(struct issue),SEEK_CUR);
            write(fd,&temp,sizeof(struct issue));
            break;
        }
    }

    if(!flag){
        int status = -1;
        write(nsd,&status,sizeof(int));
        sem_post(&books_semaphore);
        sem_post(&issues_semaphore);
        return;
    }

    struct Book newbook;
    fd=open("../db/books.bin",O_RDWR,0666);
    lseek(fd,(temp.bookid-1)*sizeof(struct Book),SEEK_SET);
    read(fd,&newbook,sizeof(struct Book));
    if(newbook.valid == 0){
        int status = -1;
        write(nsd,&status,sizeof(int));
        sem_post(&books_semaphore);
        sem_post(&issues_semaphore);
        return;
    }
    newbook.copies = newbook.copies + 1;
    lseek(fd,(temp.bookid-1)*sizeof(struct Book),SEEK_SET);
    write(fd,&newbook,sizeof(struct Book));
    close(fd);

    int status = 1;
    write(nsd,&status,sizeof(int));

    sem_post(&issues_semaphore);
    sem_post(&books_semaphore);
}

// Function to send all issued books to the client
void send_issues(int nsd){
    sem_wait(&issues_semaphore);

    struct issue temp;
    int fd=open("../db/issues.bin",O_RDWR,0666);
    int cnt = lseek(fd,0,SEEK_END)/sizeof(struct issue);
    write(nsd,&cnt,sizeof(int));
    
    for(int i=0;i<cnt;i++){
        lseek(fd,i*sizeof(struct issue),SEEK_SET);
        read(fd,&temp,sizeof(struct issue));

        write(nsd,&temp,sizeof(struct issue));
    }
    close(fd);

    sem_post(&issues_semaphore);
}

// Function to handle admin mode operations
void admin_mode(int nsd){
    while(1){

        int choice;
        read(nsd,&choice,sizeof(choice));
        printf("Choice: %d\n",choice);

        if(choice==1){
            add_book(nsd);
        }
        else if(choice==2){
            delete_book(nsd);
        }
        else if(choice==3){
            modify_book(nsd);
        }
        else if(choice==4){
            add_user(nsd);
        }
        else if(choice==5){
            send_books(nsd);
        }
        else if(choice == 6){
            send_users(nsd);
        }
        else return;
    }
}

// Function to handle user mode operations
void user_mode(int nsd){
    while(1){

        int choice;
        read(nsd,&choice,sizeof(choice));
        printf("Choice: %d\n",choice);

        if(choice==1){
            send_books(nsd);
        }
        else if(choice==2){
            issue_book(nsd);
        }
        else if(choice == 3){
            return_book(nsd);
        }
        else if(choice == 4){
            send_issues(nsd);
        }
        else return;
    }
}

// Function to handle each client connection
void *handleThread(void *args){

    int nsd=*((int *)args);
    write(nsd,"Connected to server\n",sizeof("Connected to server\n"));

    struct User temp;
    read(nsd,&temp,sizeof(struct User));

    printf("Adminame or Username: %s\n",temp.username);
    printf("Password: %s\n",temp.password);

    struct flock lock;
    lock.l_type=F_RDLCK;
    lock.l_whence=SEEK_SET;
    lock.l_start=0;
    lock.l_len=0;
    lock.l_pid=getpid();

    int fd=open("../db/users.bin",O_RDONLY,0666);
    fcntl(fd,F_SETLKW,&lock);

    struct User u;
    int flag=0;
    while(read(fd,&u,sizeof(u))){
        if(strcmp(u.username,temp.username)==0){
            flag=1;
            break;
        }
    }

    lock.l_type=F_UNLCK;
    fcntl(fd,F_SETLKW,&lock);

    int ok;
    if(strcmp(u.password,temp.password) || !flag) ok=-1;
    else ok=u.type;

    write(nsd,&ok,sizeof(int));
    
    if(ok==0) admin_mode(nsd);
    else if(ok==1) user_mode(nsd);

    return NULL;
}

// Main function
int main(){

    // Initialize semaphores
    sem_init(&books_semaphore, 0, 1);
    sem_init(&issues_semaphore, 0, 1);
    sem_init(&users_semaphore, 0, 1);

    // Declare variables for socket
    int sd,nsd;
    socklen_t len;
    struct sockaddr_in serv,cli;
    sd=socket(AF_INET,SOCK_STREAM,0);
    bzero(&serv, sizeof(serv)); 
    serv.sin_family=AF_INET;
    serv.sin_addr.s_addr=htonl(INADDR_ANY);
    serv.sin_port=htons(PORT);

    bind(sd,(struct sockaddr*)&serv,sizeof(serv));

    listen(sd,50);

    printf("Server listening.. on port %d\n",PORT);
    
    while(1){
        len=sizeof(cli);
        nsd=accept(sd,(struct sockaddr*)&cli,&len);
        pthread_t tid;
        pthread_create(&tid,NULL,handleThread,(void*)&nsd); 
    }

    // Destroy semaphores
    sem_destroy(&books_semaphore);
    sem_destroy(&issues_semaphore);
    sem_destroy(&users_semaphore);

    return 0;
}
