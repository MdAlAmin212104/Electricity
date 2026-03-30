#include <stdio.h>

#define MAX_METERS 5

// Meter Structure
struct Meter {
    int meter_id;
    float prev_reading;
    float curr_reading;
};

// Customer Structure
struct Customer {
    int customer_id;
    char name[50];
    char address[100];
    int meter_count;
    struct Meter meters[MAX_METERS];
};

// Global variable (for now only 1 customer)
struct Customer customer;

// Function to Add Customer
void addCustomer() {
    printf("\n--- Add New Customer ---\n");

    printf("Enter Customer ID: ");
    scanf("%d", &customer.customer_id);

    printf("Enter Name: ");
    scanf(" %[^\n]", customer.name);

    printf("Enter Address: ");
    scanf(" %[^\n]", customer.address);

    printf("Enter Number of Meters (Max %d): ", MAX_METERS);
    scanf("%d", &customer.meter_count);

    if (customer.meter_count > MAX_METERS) {
        printf("Too many meters! Setting to max (%d)\n", MAX_METERS);
        customer.meter_count = MAX_METERS;
    }

    for (int i = 0; i < customer.meter_count; i++) {
        printf("\nMeter %d\n", i + 1);

        printf("Enter Meter ID: ");
        scanf("%d", &customer.meters[i].meter_id);

        printf("Enter Previous Reading: ");
        scanf("%f", &customer.meters[i].prev_reading);

        printf("Enter Current Reading: ");
        scanf("%f", &customer.meters[i].curr_reading);
    }

    printf("\nCustomer Added Successfully!\n");
}

// Function to View Customer
void viewCustomer() {
    printf("\n--- Customer Details ---\n");

    printf("ID: %d\n", customer.customer_id);
    printf("Name: %s\n", customer.name);
    printf("Address: %s\n", customer.address);

    for (int i = 0; i < customer.meter_count; i++) {
        printf("\nMeter %d\n", i + 1);
        printf("Meter ID: %d\n", customer.meters[i].meter_id);
        printf("Previous Reading: %.2f\n", customer.meters[i].prev_reading);
        printf("Current Reading: %.2f\n", customer.meters[i].curr_reading);
    }
}

// Main Function
int main() {
    int choice;

    while (1) {
        printf("\n====== Electricity Bill System ======\n");
        printf("1. Add Customer\n");
        printf("2. View Customer\n");
        printf("3. Exit\n");
        printf("Enter your choice: ");
        scanf("%d", &choice);

        switch (choice) {
            case 1:
                addCustomer();
                break;
            case 2:
                viewCustomer();
                break;
            case 3:
                printf("Exiting Program...\n");
                exit(0);
            default:
                printf("Invalid Choice! Try again.\n");
        }
    }

    return 0;
}