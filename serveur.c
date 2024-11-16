#include <stdio.h>
#include <winsock2.h>
#include <stdlib.h>
#include <string.h>

#pragma comment(lib, "ws2_32.lib")

typedef struct
{
    char username[20];
    int password;
} Login;

typedef struct 
{
    char nom[50];
    char prenom[50];
    char gsm[30];
    char email[50];
    char rue[100];
    char ville[50];
    char pays[50];
} contact;

int authenticateUser(Login receivedLogin)
{
    FILE *loginFile;
    Login storedLogin;
    char typeUser[20]; 
    int userType = -1;      

    //Login file 
    loginFile = fopen("login.txt", "r");
    if (loginFile == NULL)
    {
        printf("Failed to open login file.\n");
        return -1; // Indicate authentication failure
    }

    // Compare received login with stored logins
    while (fscanf(loginFile, "%s %d %s", storedLogin.username, &storedLogin.password, typeUser) == 3)
    {
        if (strcmp(receivedLogin.username, storedLogin.username) == 0 && receivedLogin.password == storedLogin.password)
        {
            
            if (strcmp(typeUser, "admin") == 0)
            {
                userType = 1; 
            }
            else if (strcmp(typeUser, "invite") == 0)
            {
                userType = 2; 
            }
            else
            {
                userType = 0; 
            }
            break;
        }
    }

    // Close login file
    fclose(loginFile);

    return userType; // Return the user type
}

void addContact(SOCKET client, contact nvC)
{

    // Traiter les informations du contact reçues
    printf("Nouveau contact ajouté :\n");
    printf("Nom : %s\n", nvC.nom);
    printf("Prenom : %s\n", nvC.prenom);
    printf("GSM : %s\n", nvC.gsm);
    printf("Email : %s\n", nvC.email);
    printf("Adresse :\n");
    printf("  Rue : %s\n", nvC.rue);
    printf("  Ville : %s\n", nvC.ville);
    printf("  Pays : %s\n", nvC.pays);

    // Ouvrir le fichier contacts.txt en mode ajout
    FILE *f;
    f = fopen("contacts.txt", "a");
    if (f != NULL)
    {
        fprintf(f, "%s %s %s %s %s %s %s\n", nvC.nom, nvC.prenom, nvC.gsm, nvC.email, nvC.rue, nvC.ville, nvC.pays);
        fclose(f);
        // Envoi de la confirmation de l'ajout au client
        char confirmation[] = "Contact ajouté avec succès.";
        send(client, confirmation, strlen(confirmation) + 1, 0);
    }
    else
    {
        //  cas d'échec de l'ouverture du fichier
        char erreur[] = "Erreur : fichier introuvable!!";
        send(client, erreur, sizeof(erreur), 0);
    }
}
void Rechercher(SOCKET client, char *temp)
{
    contact *c;
    int counter = 0;
    contact tmp;
    //stocker temporairement les informations
    // Ouverture du fichier de contacts 
    int taille;
    FILE *f = fopen("contacts.txt", "r");
    if (f == NULL)
    {
        printf("Erreur lors de l'ouverture du fichier de contacts.\n");
        char buffer[] = "Erreur lors de l'ouverture du fichier de contacts.";
        send(client, buffer, strlen(buffer) + 1, 0);
        return;
    }

    // int array[taille];

    // 
    // arr[]
    int found = 0;
    while (fscanf(f, "%s %s %s %s %s %s %s", tmp.nom, tmp.prenom, tmp.gsm, tmp.email, tmp.rue, tmp.ville, tmp.pays) != EOF)
    {
        if (strcmp(temp, tmp.email) == 0 || strcmp(temp, tmp.gsm) == 0 || strcmp(temp, tmp.nom) == 0 || strcmp(temp, tmp.prenom) == 0)
        {
            counter++;
        }
    }

    send(client, (char *)&counter, sizeof(int), 0);

    c = malloc(sizeof(contact) * (counter + 1));
    if (!c)
        return;

    int i = 0;
    fseek(f, 0, SEEK_SET);
    i = 0;
    while (fscanf(f, "%s %s %s %s %s %s %s", tmp.nom, tmp.prenom, tmp.gsm, tmp.email, tmp.rue, tmp.ville, tmp.pays) != EOF)
    {
        if (strcmp(temp, tmp.email) == 0 || strcmp(temp, tmp.gsm) == 0 || strcmp(temp, tmp.nom) == 0 || strcmp(temp, tmp.prenom) == 0)
        {
            strcpy(c[i].email, tmp.email);
            strcpy(c[i].nom, tmp.nom);
            strcpy(c[i].prenom, tmp.prenom);
            strcpy(c[i].gsm, tmp.gsm);
            strcpy(c[i].ville, tmp.ville);
            strcpy(c[i].rue, tmp.rue);
            strcpy(c[i].pays, tmp.pays);
            i++;
        }
    }
    fclose(f);

    // Envoi du résultat de la recherche au client
    if (counter != 0)
    {
        // Envoi de la structure contact
        send(client, (char *)c, sizeof(contact) * counter, 0);
    }
    else
    {
        // Si le contact n'est pas trouvé, envoyer une structure contact vide avec un email vide
        strcpy(c[0].email, "");
        send(client, (char *)&c, sizeof(contact), 0);
    }
}

void supprimerContact(SOCKET client, char *email)
{
    // Ouvrir le fichier de contacts en mode lecture et écriture
    FILE *f = fopen("contacts.txt", "r+");
    if (f == NULL)
    {
        printf("Erreur lors de l'ouverture du fichier de contacts.\n");
        char buffer[] = "Erreur lors de l'ouverture du fichier de contacts.";
        send(client, buffer, strlen(buffer) + 1, 0);
        return;
    }

    // Créer un fichier temporaire pour stocker les contacts sans celui à supprimer
    FILE *temp = fopen("temp.txt", "w");
    if (temp == NULL)
    {
        printf("Erreur lors de la création du fichier temporaire.\n");
        char buffer[] = "Erreur lors de la création du fichier temporaire.";
        send(client, buffer, strlen(buffer) + 1, 0);
        fclose(f);
        return;
    }

    contact c;
    int found = 0;
    // Parcourir le fichier de contacts
    while (fscanf(f, "%s %s %s %s %s %s %s", c.nom, c.prenom, c.gsm, c.email, c.rue, c.ville, c.pays) != EOF)
    {
        // Vérifier si l'email correspond au contact à supprimer
        if (strcmp(email, c.email) == 0)
        {
            found = 1;
            // Ne pas écrire ce contact dans le fichier temporaire (c'est comme le supprimer)
        }
        else
        {
            // Écrire les autres contacts dans le fichier temporaire
            fprintf(temp, "%s %s %s %s %s %s %s\n", c.nom, c.prenom, c.gsm, c.email, c.rue, c.ville, c.pays);
        }
    }

    // Fermer les fichiers
    fclose(f);
    fclose(temp);

    // Remplacer le fichier original par le fichier temporaire
    if (remove("contacts.txt") != 0 || rename("temp.txt", "contacts.txt") != 0)
    {
        printf("Erreur lors de la suppression du contact.\n");
        char buffer[] = "Erreur lors de la suppression du contact.";
        send(client, buffer, strlen(buffer) + 1, 0);
        return;
    }

    // Envoyer une confirmation au client
    if (found)
    {
        char buffer[] = "Contact supprimé avec succès.";
        send(client, buffer, strlen(buffer) + 1, 0);
    }
    else
    {
        char buffer[] = "Le contact n'a pas été trouvé.";
        send(client, buffer, strlen(buffer) + 1, 0);
    }
}

void afficherTousContacts(SOCKET client) {
    contact c;
    char buffer[1024]; // Buffer for sending messages to the client

    // Open the contacts file for reading
    FILE *f = fopen("contacts.txt", "r");
    if (f == NULL) {
        printf("Erreur lors de l'ouverture du fichier de contacts.\n");
        strcpy(buffer, "Erreur lors de l'ouverture du fichier de contacts.");
        send(client, buffer, sizeof(buffer), 0);
        return;
    }

    // Send each contact to the client
    while (fscanf(f, "%s %s %s %s %s %s %s", c.nom, c.prenom, c.gsm, c.email, c.rue, c.ville, c.pays) != EOF) {
        // Send the contact structure to the client
        send(client, (char*)&c, sizeof(contact), 0);

        // Wait for acknowledgment from the client
        if (recv(client, buffer, sizeof(buffer), 0) == SOCKET_ERROR) {
            printf("Erreur lors de la réception de l'accusé de réception du client.\n");
            break;
        }
    }

    // Send a special contact or message to signal the end of contacts
    strcpy(c.nom, "Fin");
    send(client, (char*)&c, sizeof(contact), 0);

    // Close the contacts file
    fclose(f);
}

void Modifier(SOCKET client) {
    char email[30]; // Email du contact � modifier
    contact nouveau_contact; // Nouvelles informations du contact
    char reponse[100]; // R�ponse � envoyer au client
    FILE *f, *temp;
    contact c;

    // R�ception de l'email du client
    if (recv(client, email, sizeof(email), 0) == SOCKET_ERROR) {
        printf("Erreur lors de la r�ception de l'email de modification.\n");
        strcpy(reponse, "Erreur lors de la r�ception de l'email de modification.");
        send(client, reponse, sizeof(reponse), 0);
        return;
    }

    // R�ception des nouvelles informations du client
    if (recv(client, (char*)&nouveau_contact, sizeof(contact), 0) == SOCKET_ERROR) {
        printf("Erreur lors de la r�ception des nouvelles informations du contact.\n");
        strcpy(reponse, "Erreur lors de la r�ception des nouvelles informations du contact.");
        send(client, reponse, sizeof(reponse), 0);
        return;
    }

    // Ouverture du fichier original en lecture
    f = fopen("contacts.txt", "r");
    if (f == NULL) {
        printf("Erreur lors de l'ouverture du fichier de contacts.\n");
        strcpy(reponse, "Erreur lors de l'ouverture du fichier de contacts.");
        send(client, reponse, sizeof(reponse), 0);
        return;
    }

    // Ouverture d'un fichier temporaire en �criture
    temp = fopen("temp.txt", "w");
    if (temp == NULL) {
        printf("Erreur lors de l'ouverture du fichier temporaire.\n");
        strcpy(reponse, "Erreur lors de l'ouverture du fichier temporaire.");
        send(client, reponse, sizeof(reponse), 0);
        fclose(f);
        return;
    }

    // Lecture du fichier original ligne par ligne
    while (fscanf(f, "%s %s %s %s %s %s %s", c.nom, c.prenom, c.gsm, c.email, c.rue, c.ville, c.pays) != EOF) {
        // Si l'email du contact correspond � celui � modifier, �crire les nouvelles informations dans le fichier temporaire
        if (strcmp(email, c.email) == 0) {
            fprintf(temp, "%s %s %s %s %s %s %s\n", nouveau_contact.nom, nouveau_contact.prenom, nouveau_contact.gsm, nouveau_contact.email, nouveau_contact.rue, nouveau_contact.ville, nouveau_contact.pays);
        } else {
            // Sinon, �crire le contact sans modification
            fprintf(temp, "%s %s %s %s %s %s %s\n", c.nom, c.prenom, c.gsm, c.email, c.rue, c.ville, c.pays);
        }
    }

    // Fermeture des fichiers
    fclose(f);
    fclose(temp);

    // Suppression du fichier original
    remove("contacts.txt");
    // Renommage du fichier temporaire en fichier original
    rename("temp.txt", "contacts.txt");

    // Envoi de la r�ponse au client
    strcpy(reponse, "Modification effectu�e avec succ�s.");
    send(client, reponse, sizeof(reponse), 0);
}
#define PORT 50000
#define MAX_LOGIN_ATTEMPTS 3

int main()
{
    WSADATA wsa;
    SOCKET server, client;
    struct sockaddr_in serverAddr, clientAddr;
    Login receivedLogin, storedLogin;
    int userType = 0;
    int choice;
    int loginAttempts = 0; // compteur de tentatives
    contact nvC;
    char email[30]; // Email du contact à rechercher
    contact c;    // Variable pour stocker temporairement les informations du contact trouvé
    char emailToDelete[50];

    // Initialize Winsock
    if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0)
    {
        printf("Failed to initialize Winsock.\n");
        return 1;
    }

    // Create socket
    if ((server = socket(AF_INET, SOCK_STREAM, 0)) == INVALID_SOCKET)
    {
        printf("Failed to create socket.\n");
        WSACleanup();
        return 1;
    }

    // Server address
    memset(&serverAddr, 0, sizeof(serverAddr));
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_addr.s_addr = htonl(INADDR_LOOPBACK);
    serverAddr.sin_port = htons(PORT);
    

    // Bind socket
    if (bind(server, (struct sockaddr *)&serverAddr, sizeof(serverAddr)) == SOCKET_ERROR)
    {
        printf("Failed to bind socket.\n");
        closesocket(server);
        WSACleanup();
        return 1;
    }

    // Listen for connections
    if (listen(server, 5) == SOCKET_ERROR)
    {
        printf("Failed to listen.\n");
        closesocket(server);
        WSACleanup();
        return 1;
    }

    printf("Server is listening on port %d...\n", PORT);

    // Accept connection
    int addrLen = sizeof(clientAddr);
    if ((client = accept(server, (struct sockaddr *)&clientAddr, &addrLen)) == INVALID_SOCKET)
    {
        printf("Failed to accept connection.\n");
        closesocket(server);
        WSACleanup();
        return 1;
    }

    printf("Client connected.\n");

    // Authentifier l'utilisateur
    while (loginAttempts < MAX_LOGIN_ATTEMPTS)
    {
        // Receive login info from client
        if (recv(client, (char *)&receivedLogin, sizeof(Login), 0) == SOCKET_ERROR)
        {
            printf("Error receiving login info from client.\n");
            closesocket(client);
            closesocket(server);
            WSACleanup();
            return 1;
        }
        FILE *loginFile;
        // Open login file
        loginFile = fopen("login.txt", "r");
        if (loginFile == NULL)
        {
            printf("Failed to open login file.\n");
            closesocket(client);
            closesocket(server);
            WSACleanup();
            return 1;
        }

        // Compare received login with stored logins
        char typeUser[20]; // Store the user type as a string
        int found = 0;
        while (fscanf(loginFile, "%s %d %s", storedLogin.username, &storedLogin.password, typeUser) == 3)
        {
            if (strcmp(receivedLogin.username, storedLogin.username) == 0 && receivedLogin.password == storedLogin.password)
            {
                // If login matches, set userType and indicate that a match is found
                found = 1;
                break;
            }
        }

        // Close login file
        fclose(loginFile);

        // Send response to client
        if (found)
        {
            if (strcmp(typeUser, "admin") == 0)
            {
                // If user is an admin, send 1 to client
                userType = 1;
            }
            else if (strcmp(typeUser, "invite") == 0)
            {
                // If user is an invitee, send 2 to client
                userType = 2;
            }
            send(client, (char *)&userType, sizeof(int), 0);

            // If login is successful, break out of the loop
            break;
        }
        else
        {
            // If no matching login is found, send 0 to client
            userType = 0;
            send(client, (char *)&userType, sizeof(int), 0);

            // Increment login attempts
            loginAttempts++;

            // If maximum login attempts reached, terminate connection
            if (loginAttempts >= MAX_LOGIN_ATTEMPTS)
            {
                printf("Maximum login attempts reached. Closing connection.\n");
                closesocket(client);
                closesocket(server);
                WSACleanup();
                return 1;
            }
        }
    }

    printf("Utilisateur authentifié.\n");

    // Boucle pour gérer les demandes des clients
    while (1)
    {
        // Recevoir le choix de l'utilisateur
        if (recv(client, (char *)&choice, sizeof(int), 0) == SOCKET_ERROR)
        {
            printf("Erreur lors de la réception du choix de l'utilisateur.\n");
            break; // Sortir de la boucle si une erreur de réception se produit
        }

        // Traiter la demande en fonction du choix de l'utilisateur et de son type
        switch (userType)
        {
        case 1: // Administrateur
            switch (choice)
            {
            case 1:
                // Recevoir les données du contact à ajouter
                if (recv(client, (char *)&nvC, sizeof(contact), 0) == SOCKET_ERROR)
                {
                    printf("Erreur lors de la réception des données du contact à ajouter.\n");
                    break;
                }
                addContact(client, nvC);
                break;
            case 2:
                // Réception de l'email du client
                if (recv(client, email, sizeof(email), 0) == SOCKET_ERROR)
                {
                    printf("Erreur lors de la réception de l'email de recherche.\n");
                    char buffer[] = "Erreur lors de la réception de l'email de recherche.";
                    send(client, buffer, strlen(buffer) + 1, 0);
                    break;
                }
                Rechercher(client, email);
                break;
            case 3:
                // Supprimer un contact
                // Recevoir l'email du contact à supprimer du client
                if (recv(client, emailToDelete, sizeof(emailToDelete), 0) == SOCKET_ERROR)
                {
                    printf("Erreur lors de la réception de l'email de suppression du client.\n");
                    break;
                }
                // Appeler une fonction pour supprimer le contact
                supprimerContact(client, emailToDelete);
                break;
            case 4:
                Modifier(client);
                        break;
            case 5:
                afficherTousContacts(client);
                break;
            case 6:
                // Déconnecter le client et sortir de la boucle
                printf("Déconnexion demandée. Fermeture de la connexion.\n");
                closesocket(client);
                break;

            }
            break;
        case 2: // Utilisateur régulier
            switch (choice)
            {
            case 1:
    // Appeler la fonction de recherche pour les utilisateurs
                if (recv(client, email, sizeof(email), 0) == SOCKET_ERROR)
                        {
                        printf("Erreur lors de la réception de l'email de recherche.\n");
                        char buffer[] = "Erreur lors de la réception de l'email de recherche.";
                        send(client, buffer, strlen(buffer) + 1, 0);
                        break;
                        }
                    Rechercher(client, email);
                    break;
            case 2:
                afficherTousContacts(client);
                break;
            case 3:
                // Déconnecter le client et sortir de la boucle
                printf("Déconnexion demandée. Fermeture de la connexion.\n");
                closesocket(client);
                break;
           
            }
            break;
        default:
            printf("Utilisateur non authentifié.\n");
            break;
        }
    }

    // Fermeture du socket et nettoyage
    closesocket(client);
    closesocket(server);
    WSACleanup();
    return 0;
}
