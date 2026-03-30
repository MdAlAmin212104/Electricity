/*
 * ================================================
 *    ELECTRICITY BILL MANAGEMENT SYSTEM
 *    Version: 40%
 *    Features: Add / View / Calculate / Exit
 * ================================================
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_METERS     5
#define MAX_CUSTOMERS  100
#define RATE           5.5f
#define SERVICE_CHARGE 50.0f

/* ── Structures ── */
struct Meter {
    int   meter_id;
    float prev_reading;
    float curr_reading;
    float units;
    float bill;
};

struct Customer {
    int          customer_id;
    char         name[50];
    char         address[100];
    int          meter_count;
    struct Meter meters[MAX_METERS];
};

/* ── Global Array ── */
struct Customer customers[MAX_CUSTOMERS];
int count = 0;

/* ================================================
   FIND CUSTOMER BY ID
   ================================================ */
int findCustomer(int id) {
    for (int i = 0; i < count; i++)
        if (customers[i].customer_id == id)
            return i;
    return -1;
}

/* ================================================
   1. ADD CUSTOMER
   ================================================ */
void addCustomer() {
    if (count >= MAX_CUSTOMERS) {
        printf("\nCustomer limit reached!\n");
        return;
    }

    printf("\n--- Add New Customer ---\n");

    int id;
    printf("Enter Customer ID: ");
    if (scanf("%d", &id) != 1) {
        while (getchar() != '\n');
        printf("[ERROR] Invalid ID!\n");
        return;
    }

    /* Duplicate ID check */
    if (findCustomer(id) != -1) {
        printf("[ERROR] Customer ID %d already exists!\n", id);
        while (getchar() != '\n');
        return;
    }

    customers[count].customer_id = id;
    while (getchar() != '\n');

    printf("Enter Name: ");
    fgets(customers[count].name, 50, stdin);
    customers[count].name[strcspn(customers[count].name, "\n")] = '\0';

    printf("Enter Address: ");
    fgets(customers[count].address, 100, stdin);
    customers[count].address[strcspn(customers[count].address, "\n")] = '\0';

    int mc;
    printf("Enter Number of Meters (Max %d): ", MAX_METERS);
    if (scanf("%d", &mc) != 1 || mc < 1 || mc > MAX_METERS) {
        printf("[ERROR] Invalid meter count! Must be 1 to %d.\n", MAX_METERS);
        while (getchar() != '\n');
        return;
    }
    customers[count].meter_count = mc;

    for (int i = 0; i < mc; i++) {
        printf("\nMeter %d\n", i + 1);

        printf("  Enter Meter ID        : ");
        scanf("%d", &customers[count].meters[i].meter_id);

        printf("  Enter Previous Reading: ");
        scanf("%f", &customers[count].meters[i].prev_reading);

        printf("  Enter Current  Reading: ");
        scanf("%f", &customers[count].meters[i].curr_reading);

        /* Validate reading */
        if (customers[count].meters[i].curr_reading <
            customers[count].meters[i].prev_reading) {
            printf("  [WARNING] Current < Previous. Units set to 0.\n");
            customers[count].meters[i].units = 0;
        } else {
            customers[count].meters[i].units =
                customers[count].meters[i].curr_reading -
                customers[count].meters[i].prev_reading;
        }

        customers[count].meters[i].bill = 0;
    }

    count++;
    printf("\nCustomer Added Successfully!\n");
}

/* ================================================
   2. VIEW ALL CUSTOMERS
   ================================================ */
void viewAllCustomers() {
    if (count == 0) {
        printf("\nNo customers found!\n");
        return;
    }

    for (int i = 0; i < count; i++) {
        printf("\n=========================\n");
        printf("ID     : %d\n",  customers[i].customer_id);
        printf("Name   : %s\n",  customers[i].name);
        printf("Address: %s\n",  customers[i].address);
        printf("Meters : %d\n",  customers[i].meter_count);

        for (int j = 0; j < customers[i].meter_count; j++) {
            printf("\n  Meter %d (ID: %d)\n",    j + 1, customers[i].meters[j].meter_id);
            printf("    Prev Reading: %.2f\n",   customers[i].meters[j].prev_reading);
            printf("    Curr Reading: %.2f\n",   customers[i].meters[j].curr_reading);
            printf("    Units       : %.2f\n",   customers[i].meters[j].units);
            printf("    Bill        : %.2f\n",   customers[i].meters[j].bill);
        }
        printf("=========================\n");
    }
}

/* ================================================
   3. CALCULATE BILL
   ================================================ */
void calculateBill() {
    printf("\nEnter Customer ID to calculate bill: ");
    int id;
    if (scanf("%d", &id) != 1) {
        while (getchar() != '\n');
        printf("[ERROR] Invalid input!\n");
        return;
    }

    int idx = findCustomer(id);
    if (idx == -1) {
        printf("Customer not found!\n");
        return;
    }

    float total = 0;
    printf("\n--- Bill Details ---\n");

    for (int j = 0; j < customers[idx].meter_count; j++) {
        struct Meter *m = &customers[idx].meters[j];

        m->units = m->curr_reading - m->prev_reading;

        if (m->units < 0) {
            printf("Invalid readings for Meter %d\n", j + 1);
            m->units = 0;
            m->bill  = 0;
            continue;
        }

        m->bill = m->units * RATE;
        total  += m->bill;

        printf("\nMeter %d (ID: %d)\n",  j + 1, m->meter_id);
        printf("  Units: %.2f\n",        m->units);
        printf("  Bill : BDT %.2f\n",    m->bill);
    }

    total += SERVICE_CHARGE;
    printf("\nService Charge : BDT %.2f\n", SERVICE_CHARGE);
    printf("Total Bill     : BDT %.2f\n",   total);
}

/* ================================================
   MAIN MENU
   ================================================ */
int main() {
    int choice;

    while (1) {
        printf("\n====== Electricity System (40%%) ======\n");
        printf("1. Add Customer\n");
        printf("2. View Customers\n");
        printf("3. Calculate Bill\n");
        printf("4. Exit\n");
        printf("Enter choice: ");

        if (scanf("%d", &choice) != 1) {
            while (getchar() != '\n');
            printf("Invalid input!\n");
            continue;
        }

        switch (choice) {
            case 1: addCustomer();      break;
            case 2: viewAllCustomers(); break;
            case 3: calculateBill();    break;
            case 4:
                printf("Goodbye!\n");
                exit(0);
            default:
                printf("Invalid choice!\n");
        }
    }

    return 0;
}
