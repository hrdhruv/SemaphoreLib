#include"../header/head.h" // Include necessary header file

// Function to handle administrative actions
bool handleAdmin(int sd){
    // Display menu options
    printf("Choose option:\n");
    printf("1. Add new book.\n");
    printf("2. Delete a book.\n");
    printf("3. Update a book.\n");
    printf("4. Add new user.\n");
    printf("5. Show all available books\n");
    printf("6. Show all users and admins\n");
    printf("7. Logout \n");
    
    // Get user response
    int response;
    scanf("%d",&response);
    write(sd,&response,sizeof(int)); // Send response to server

    // Execute corresponding action based on user response
    if(response==1){
        // Add new book
        struct Book b; 
        printf("Enter name of the book: ");
        scanf("%s",b.title);
        printf("Enter name of Author: ");
        scanf("%s",b.author);
        printf("Enter number of copies available: ");
        scanf("%d",&b.copies);
        b.id=-1;
        b.valid = true;
        write(sd,&b,sizeof(b)); // Send book information to server
    }
    else if(response == 2){
        // Delete a book
        printf("Enter ID of the book to delete: ");
        int id = -1;
        scanf("%d",&id);
        write(sd,&id,sizeof(int)); // Send book ID to server
    }
    else if(response == 3){
        // Update a book
        printf("Enter ID of the book to modify: ");
        int id = -1;
        scanf("%d",&id);
        printf("Enter the new number of copies available: ");
        int nc = -1;
        scanf("%d",&nc);
        write(sd,&id,sizeof(int)); // Send book ID to server
        write(sd,&nc,sizeof(int)); // Send new copies count to server
    }
    else if(response == 4){
        // Add new user
        struct User u;
        printf("Enter username: ");
        scanf("%s",u.username);
        printf("Enter password: ");
        scanf("%s",u.password);
        u.type = 1;
        write(sd,&u,sizeof(struct User)); // Send new user information to server
    }
    else if(response == 5){ 
        // Show all available books
        int cnt = 0;
        read(sd,&cnt,sizeof(int)); // Receive book count from server
        struct Book temp;
        for(int i=0;i<cnt;i++){
            read(sd,&temp,sizeof(struct Book)); // Receive book details from server
            if(! temp.valid) continue;
            printf("Title: %s\n",temp.title);
            printf("Author: %s\n",temp.author);
            printf("Copies: %d\n",temp.copies);
            printf("Valid: %d\n",temp.valid);
            printf("ID: %d\n",temp.id);
            printf("\n");
        }
    }
    else if(response == 6){
        // Show all users and admins
        int cnt = 0;
        read(sd,&cnt,sizeof(int)); // Receive user count from server
        printf("%d\n",cnt);
        struct User temp;
        for(int i=0;i<cnt;i++){
            read(sd,&temp,sizeof(struct User)); // Receive user details from server
            printf("Username: %s\n",temp.username);
            printf("Role: %s\n",temp.type ? "User":"Admin");
        }
    }
    else return false;
    return true;
}

// Function to handle user actions
bool handleUser(int sd, char * username ){
    // Display menu options
    printf("Choose option:\n");
    printf("1. Check all available books.\n");
    printf("2. Issue book \n");
    printf("3. Return book.\n");
    printf("4. Check all books issued by %s\n",username);
    printf("5. Logout \n");

    // Get user response
    int response;
    scanf("%d",&response);
    write(sd,&response,sizeof(int)); // Send response to server

    // Execute corresponding action based on user response
    if(response == 1){
        // Check all available books
        int cnt = 0;
        read(sd,&cnt,sizeof(int)); // Receive book count from server
        struct Book temp;
        for(int i=0;i<cnt;i++){
            read(sd,&temp,sizeof(struct Book)); // Receive book details from server
            if(!temp.valid || temp.copies == 0) continue;
            printf("Title: %s\n",temp.title);
            printf("Author: %s\n",temp.author);
            printf("Copies: %d\n",temp.copies);
            printf("Valid: %d\n",temp.valid);
            printf("ID: %d\n",temp.id);
            printf("\n");
        }
    }
    else if(response == 2){
        // Issue book
        int bookid=-1;
        printf("Enter book id of the book you want to issue: ");
        scanf("%d",&bookid);
        struct issue is;
        is.bookid = bookid;
        strcpy(is.username,username);
        is.valid = true;
        write(sd,&is,sizeof(struct issue)); // Send issue request to server      
        int stat=0;
        read(sd,&stat,sizeof(int)); // Receive status from server
        if(stat == -1) printf("Book not available\n");
        else printf("Book issued Succesfully\n");  
    }
    else if(response == 3){
        // Return book
        int bookid=-1;
        printf("Enter book id of the book you want to return: ");
        scanf("%d",&bookid);
        struct issue is;
        is.bookid = bookid;
        strcpy(is.username,username);
        is.valid = false;
        write(sd,&is,sizeof(struct issue)); // Send return request to server

        int stat=0;
        read(sd,&stat,sizeof(int)); // Receive status from server
        if(stat == -1) printf("Book could not be returned\n");
        else printf("Book returned Succesfully\n");  
    }
    else if(response == 4){
        // Check all books issued by the user
        int cnt = 0;
        read(sd,&cnt,sizeof(int)); // Receive issue count from server
        struct issue is;
        for(int i=0;i<cnt;i++){
            read(sd,&is,sizeof(struct issue)); // Receive issue details from server
            if(is.valid && !strcmp(is.username,username)){
                printf("Username of borrower: %s\n",is.username);
                printf("Id of Book issued: %d\n",is.bookid);
            }
        }
    }
    else return false;
    return true;
}

// Main function
int main(){
    struct sockaddr_in server;
    int sd = socket(AF_INET,SOCK_STREAM,0); // Create socket

    // Server address setup
    server.sin_addr.s_addr = inet_addr("127.0.0.1");
    server.sin_family = AF_INET;
    server.sin_port = htons(PORT);

    // Connect to server
    int stat = connect(sd,(struct sockaddr*)&server,sizeof(server));
        if(stat<0){
        perror("Connect");
        return 0;
    }

    char buf[BUFSIZ];
    read(sd,buf,BUFSIZ); // Receive welcome message from server
    printf("%s\n",buf);

    struct User temp;
    printf("Enter username: ");
    scanf("%s",temp.username);
    printf("Enter Password: ");
    scanf("%s",temp.password);
    temp.type = 0;

    int authstat = 0;

    write(sd,&temp,sizeof(temp)); // Send user credentials to server
    read(sd,&authstat,sizeof(int)); // Receive authentication status from server

    if(authstat == 0){
        printf("\nLogged in Successfully as Admin\n\n");
        bool br = true;
        while(br){
            br = handleAdmin(sd); // Handle admin actions
        }
    } 
    else if(authstat == 1){
        printf("Logged in Successfully as user %s\n",temp.username);
        bool br = true;
        while(br){
            br = handleUser(sd,temp.username); // Handle user actions
        }
    }
    else {
        printf("Authentication Failed\n");
        return -1;
    }
}
