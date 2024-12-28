#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <pthread.h>
#include <time.h>

#define AF_BP 28 // BP protocol family
#define MAX_MESSAGE_LEN 256
#define MAX_CHAT_HISTORY 20
#define LOCAL_EID "ipn:1.0"
#define DEST_EID "ipn:2.1"

// Global variables
char chat_history[MAX_CHAT_HISTORY][MAX_MESSAGE_LEN + 50]; // For timestamped messages
int history_count = 0;
pthread_mutex_t chat_lock;

// Utility: Get current timestamp
void get_timestamp(char *buffer, size_t len) {
    time_t now = time(NULL);
    struct tm *tm_info = localtime(&now);
    strftime(buffer, len, "%Y-%m-%d %H:%M:%S", tm_info);
}

// Add a message to chat history
void add_to_chat_history(const char *message) {
    pthread_mutex_lock(&chat_lock);
    if (history_count < MAX_CHAT_HISTORY) {
        strncpy(chat_history[history_count++], message, MAX_MESSAGE_LEN + 50);
    } else {
        // Shift history to make room for the new message
        memmove(chat_history, chat_history + 1, (MAX_CHAT_HISTORY - 1) * (MAX_MESSAGE_LEN + 50));
        strncpy(chat_history[MAX_CHAT_HISTORY - 1], message, MAX_MESSAGE_LEN + 50);
    }
    pthread_mutex_unlock(&chat_lock);
}

// Print chat history
void print_chat_history() {
    pthread_mutex_lock(&chat_lock);
    printf("\n=== Chat History ===\n");
    for (int i = 0; i < history_count; i++) {
        printf("%s\n", chat_history[i]);
    }
    printf("====================\n");
    pthread_mutex_unlock(&chat_lock);
}

// Receiver thread function
void *receive_messages(void *arg) {
    int sockfd = *(int *)arg;
    char buffer[MAX_MESSAGE_LEN];
    struct sockaddr src_addr;
    socklen_t addr_len = sizeof(src_addr);

    while (1) {
        ssize_t received = recvfrom(sockfd, buffer, MAX_MESSAGE_LEN, 0, &src_addr, &addr_len);
        if (received > 0) {
            buffer[received] = '\0'; // Null-terminate the received message
            char timestamp[20];
            get_timestamp(timestamp, sizeof(timestamp));

            char formatted_message[MAX_MESSAGE_LEN + 50];
            snprintf(formatted_message, sizeof(formatted_message), "[%s] %s: %s", timestamp, src_addr.sa_data, buffer);

            add_to_chat_history(formatted_message);
            printf("\n%s\n>>> ", formatted_message);
            fflush(stdout);
        } else {
            perror("Error receiving message");
        }
    }
    return NULL;
}

int main() {
    int sockfd, ret;
    struct sockaddr eid_addr;

    // Initialize chat lock
    pthread_mutex_init(&chat_lock, NULL);

    printf("Welcome to BP Messaging App!\n");
    printf("Please enter your username: ");
    char username[MAX_MESSAGE_LEN / 2];
    fgets(username, sizeof(username), stdin);
    username[strcspn(username, "\n")] = '\0'; // Remove newline character

    // Create BP socket
    sockfd = socket(AF_BP, SOCK_DGRAM, 0);
    if (sockfd < 0) {
        perror("Socket creation failed");
        return EXIT_FAILURE;
    }

    // Bind to a local EID
    struct sockaddr local_addr;
    local_addr.sa_family = AF_BP;
    strncpy(local_addr.sa_data, LOCAL_EID, sizeof(local_addr.sa_data));
    local_addr.sa_data[sizeof(local_addr.sa_data) - 1] = '\0';

    if (bind(sockfd, &local_addr, sizeof(local_addr)) < 0) {
        perror("Bind failed");
        close(sockfd);
        return EXIT_FAILURE;
    }
    printf("Bound to EID: %s\n", LOCAL_EID);

    // Prepare the destination address
    eid_addr.sa_family = AF_BP;
    strncpy(eid_addr.sa_data, DEST_EID, sizeof(eid_addr.sa_data));
    eid_addr.sa_data[sizeof(eid_addr.sa_data) - 1] = '\0';

    // Start receiving messages
    pthread_t recv_thread;
    if (pthread_create(&recv_thread, NULL, receive_messages, &sockfd) != 0) {
        perror("Failed to create receiver thread");
        close(sockfd);
        return EXIT_FAILURE;
    }

    // Main loop
    printf("Type /help for commands. Type 'exit' to quit.\n");
    while (1) {
        printf(">>> ");
        fflush(stdout);

        char message[MAX_MESSAGE_LEN];
        fgets(message, sizeof(message), stdin);
        message[strcspn(message, "\n")] = '\0'; // Remove newline character

        if (strcmp(message, "exit") == 0) {
            printf("Exiting...\n");
            break;
        } else if (strcmp(message, "/help") == 0) {
            printf("\nCommands:\n");
            printf("/help - Show this help message\n");
            printf("/clear - Clear the chat history\n");
            printf("exit - Quit the chat application\n");
        } else if (strcmp(message, "/clear") == 0) {
            pthread_mutex_lock(&chat_lock);
            history_count = 0;
            pthread_mutex_unlock(&chat_lock);
            printf("Chat history cleared.\n");
        } else {
            // Prepend username to message
            char full_message[MAX_MESSAGE_LEN];
            snprintf(full_message, sizeof(full_message), "%s: %s", username, message);

            // Send the message
            ret = sendto(sockfd, full_message, strlen(full_message), 0, &eid_addr, sizeof(eid_addr));
            if (ret < 0) {
                perror("Failed to send message");
            } else {
                char timestamp[20];
                get_timestamp(timestamp, sizeof(timestamp));
                char formatted_message[MAX_MESSAGE_LEN + 50];
                snprintf(formatted_message, sizeof(formatted_message), "[%s] You: %s", timestamp, message);
                add_to_chat_history(formatted_message);
            }
        }
    }

    // Cleanup
    pthread_cancel(recv_thread);
    pthread_join(recv_thread, NULL);
    close(sockfd);
    pthread_mutex_destroy(&chat_lock);

    return EXIT_SUCCESS;
}
